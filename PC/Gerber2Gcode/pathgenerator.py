# -*- coding: utf-8 -*-
"""
Created on Mon May 25 11:55:41 2020

@author: fredr
"""

#Importing librarys 
import numpy as np
import math
from sklearn import preprocessing

##Declering Filepaths
#filepath1 = 'SwitchingSupply-Edge_Cuts.gbr'
##filepath2 = 'SwitchingSupply-F_Paste.gbr'
#filepath2 = 'AVR proto board-F_Paste.gbr'
#filepath3 = 'testskrive.txt'

def create_G_file(filepath1, filepath2, filepath3):
   #Declering Variabels
   F = 0
   N = 0
   
   
   #Declering Arrays and lists
   edge_cut = []
   apature_list = []
   placement_list = []
   component_list = []
   free_drawing = []
   free_placement = np.zeros(shape=(1,2))
   free_apature = np.zeros(shape=(1,2))
   apature = []
   component_class = []
   current_posision = np.zeros(shape=(1,2))
   
   #Reading the Edge-Cut file and storing data
   with open (filepath1) as fp1:
       lines = fp1.readlines()
       k = 0
       while k < len(lines):
           string = lines[k]
           if string[:1] == 'X':
               edge_cut.append(string)
           k += 1
   
   #Reading the paste file and storing data
   with open (filepath2) as fp2: 
       lines = fp2.readlines()
       k = 0
       start_placement = 0
       while k < len(lines):
           string = lines[k]
           
   #Finding the Apatures for the PCB
           if '%ADD' in string: 
               if 'R' in string:
                  shape = 'R'
               if 'C' in string:
                   shape = 'C'
               apature_list.append(string)
               
   #Finding the Apature names
           if string[:1] == 'D':
               D = string.translate({ord(i): None for i in '*'})
               start_placement = 1
   
   #Starting Free drawing         
           if string[:4] == 'G36*':
               start_placement = 2
               component_list.append(D)
               
   #Ending Free drawing        
           if string[:4] == 'G37*':
               globals()['edge_free_0%d' % F] = free_drawing
               apature_list.append(['0','0'])
               component_list.append('Free{}'.format(F))
               start_placement = 1
               free_drawing = []
               F += 1
           
   #End of document 
           if string[:4] == 'M02*':
               start_placement = 0
           
   #Finding the apature placements
           if start_placement == 2:
               if string[:1] != 'D' and string[:1] != 'G':
                   free_drawing.append(string)
               
           if start_placement == 1:
               if string[:1] != 'D' and string[:1] != 'G':
                   placement_list.append(string)
                   component_list.append(D)
           k += 1
   
   
   #Converting data form Edge-Cut file
   i = 0
   while i < len(edge_cut):
       edge_cut[i] = edge_cut[i].split('D')
       edge_cut[i] = edge_cut[i][0]
       edge_cut[i] = edge_cut[i].translate({ord(i): None for i in 'X'})
       edge_cut[i] = edge_cut[i].split('Y')
       edge_cut[i] = [int(edge_cut[i][0]), int(edge_cut[i][1])]
       i += 1           

   #Finding the offset from the edge cut data 
   edge_cut = np.array(edge_cut)
   offset_x = np.argmin(np.hsplit(edge_cut,2)[0])
   offset_y = np.argmin(np.hsplit(edge_cut,2)[1])
   offset = np.array([edge_cut[offset_x][0],edge_cut[offset_y][1]])/1000000
   offset = np.array([130,-135])


   #Converting Free drawing data and storing it for later use
   i = 0
   while i < F:
       edge_free = globals()['edge_free_0%d' % i]
       j = 0
       while j < len(edge_free):
           edge_free[j] = edge_free[j].split('D')
           edge_free[j] = edge_free[j][0]
           edge_free[j] = edge_free[j].translate({ord(i): None for i in 'X'})
           edge_free[j] = edge_free[j].split('Y')
           edge_free[j] = [np.float32(edge_free[j][0]),np.float32(edge_free[j][1])]
           j += 1
       globals()['edge_free_0%d' % i] = np.unique(np.array(edge_free)/1000000 , axis = 0)
       globals()['edge_free_0%d' % i] = globals()['edge_free_0%d' % i] - offset
       min_y = np.argmin(np.hsplit(globals()['edge_free_0%d' % i],2)[1])
       max_y = np.argmax(np.hsplit(globals()['edge_free_0%d' % i],2)[1])
       min_x = np.argmin(np.hsplit(globals()['edge_free_0%d' % i],2)[0])
       max_x = np.argmax(np.hsplit(globals()['edge_free_0%d' % i],2)[0])
       free_x = globals()['edge_free_0%d' % i][min_x][0] + (globals()['edge_free_0%d' % i][max_x][0] - globals()['edge_free_0%d' % i][min_x][0])/2
       free_y = globals()['edge_free_0%d' % i][min_y][1] + (globals()['edge_free_0%d' % i][max_y][1] - globals()['edge_free_0%d' % i][min_y][1])/2
       free_placement = np.append(free_placement, [[free_x,free_y]], axis = 0)
       len_1 = math.sqrt(math.pow(globals()['edge_free_0%d' % i][min_y][0] - globals()['edge_free_0%d' % i][min_x][0],2) + math.pow(globals()['edge_free_0%d' % i][min_y][1] - globals()['edge_free_0%d' % i][min_x][1],2))
       len_2 = math.sqrt(math.pow(globals()['edge_free_0%d' % i][max_x][0] - globals()['edge_free_0%d' % i][min_y][0],2) + math.pow(globals()['edge_free_0%d' % i][max_x][1] - globals()['edge_free_0%d' % i][min_y][1],2))
       free_apature = np.append(free_apature,[[len_1 , len_2]], axis = 0)
       i += 1
   free_placement = np.delete(free_placement, 0, axis = 0)
   free_apature = np.delete(free_apature, 0, axis = 0)           
   
   
   #Converting data from apature list and combining it whit the Free drawing apatures
   i = 0
   j = 0
   n = 0
   while i < len(apature_list):
       if apature_list[i] != ['0','0']:
           apature_list[i] = apature_list[i].translate({ord(i): None for i in '%AD*'})
           if 'R' in apature_list[i]:
               apature_list[i] = apature_list[i].split('R,')
               apature.append('D{}'.format(apature_list[i][0]))
               apature_list[i] = apature_list[i][1].split('X')
               apature_list[i] = ([np.float32(apature_list[i][0]),np.float32(apature_list[i][1])])
               j += 1 
           if 'C' in apature_list[i]:
               apature_list[i] = apature_list[i].split('C,')
               apature.append('D{}'.format(apature_list[i][0]))
               apature_list[i] = [math.pow(np.float32(apature_list[i][1]),2),np.pi]
               j += 1 
       else:
           apature_list[i] = free_apature[i-j]
           apature.append('Free{}'.format(n))
           n += 1
       i += 1
   apature_list = np.array(apature_list)
   
   #Converting data from the placement list 
   i = 0
   while i < len(placement_list):
       placement_list[i] = placement_list[i].split('D')
       placement_list[i] = placement_list[i][0]
       placement_list[i] = placement_list[i].translate({ord(i): None for i in 'X'})
       placement_list[i] = placement_list[i].split('Y')
       placement_list[i] = [np.float32(placement_list[i][0]),np.float32(placement_list[i][1])]
       i += 1
   
   #Compansating for the offset in gerber file
   placement_list = np.array(placement_list)/1000000
   placement_list = placement_list - offset
   
   #Combining placement lists
   placement_list = np.append(placement_list, free_placement, axis = 0)
   
   #Calculating the area of components
   apature_x = np.hsplit(apature_list,2)[0]
   apature_y = np.hsplit(apature_list,2)[1]
   apature_area = apature_x * apature_y
   
   #Assigning classes to the components
   i = 0
   for rows in apature:
       if apature_y[i] == np.pi:
           component_class.append('circle')
       elif apature_area[i] > 3:
           component_class.append('big')
       elif apature_y[i] > apature_x[i]*2:
           component_class.append('longY')
       elif apature_x[i] > apature_y[i]*2:
           component_class.append('longX')
       else:
           component_class.append('small')
       i += 1

   #Duplicating important arrays
   place = placement_list
   component_name = list(dict.fromkeys(component_list))
   
   #Calculating the path trough the PCB
   fp3 = open(filepath3,'w+')
   fp3.write('G0')
   fp3.write('\n')
   
   for rows in place:
       place = place - current_posision
       new_place = np.empty(len(place))
       m = 0
   #Finding the next step in the path
       for rows in place:
           new_place[m] = math.sqrt(math.pow(place[m,0],2)+math.pow(place[m,1],2))
           m += 1
       P = np.argmin(new_place)
       current_posision = place[P]
       D = component_list[P]
       index = component_name.index(D)
       edge_big = np.zeros(shape=(4,2))
       edge_longx = np.zeros(shape=(2,2))
       edge_longy = np.zeros(shape=(2,2))
       big = np.zeros(shape=(4,1))
       longy = np.zeros(shape=(2,1))
       longx = np.zeros(shape=(2,1))
       o = 0
       s = 0
    
   #Big component path
       if component_class[index] == 'big':
           if component_name[index][:4] == 'Free':
               numb = int(component_name[index].translate({ord(i): None for i in 'Fre'}))
               print(numb)
               edge_big = globals()['edge_free_0%d' % numb]
               x_max = np.argmax(np.hsplit(globals()['edge_free_0%d' % numb],2)[0])
               y_max = np.argmax(np.hsplit(globals()['edge_free_0%d' % numb],2)[1])
               x_min = np.argmin(np.hsplit(globals()['edge_free_0%d' % numb],2)[0])
               y_min = np.argmin(np.hsplit(globals()['edge_free_0%d' % numb],2)[1])
               t = True
               s = [[0,0]]
               fp3.write('N{}'.format(N))
               if apature_x[index] < apature_y[index]:
                   normal = preprocessing.normalize([[edge_big[y_min][0] - edge_big[x_min][0] , edge_big[y_min][1] - edge_big[x_min][1]]]) * 0.1
                   while s[0][0] < apature_x[index]:
                       if t == True:
                           x = edge_big[y_min,0] + s[0][0]
                           y = edge_big[y_min,1] + s[0][1]
                           t = not t
                       else:
                           x = edge_big[x_max,0] + s[0][0]
                           y = edge_big[x_max,1] + s[0][1]
                           t = not t
                       fp3.write('X{}Y{}'.format(round(x,4),round(y,4)))
                       fp3.write('\n')
                       fp3.write('X{}Y{}'.format(round(x,4),round(y,4)))
                       fp3.write('\n')
                       s = s + normal
           else:                                       
               edge_big[0] = [placement_list[P,0] - (apature_x[index])/2, placement_list[P,1] - (apature_y[index])/2]
               edge_big[1] = [placement_list[P,0] - (apature_x[index])/2, placement_list[P,1] + (apature_y[index])/2]
               edge_big[2] = [placement_list[P,0] + (apature_x[index])/2, placement_list[P,1] + (apature_y[index])/2]
               edge_big[3] = [placement_list[P,0] + (apature_x[index])/2, placement_list[P,1] - (apature_y[index])/2]
               t = True
               fp3.write('N{}'.format(N))
               while s < apature_x[index]:
                   x = edge_big[0,0] + s
                   if t == True:
                       y = edge_big[0,1]
                       t = not t
                   else:
                       y = edge_big[1,1]
                       t = not t
                   fp3.write('X{}Y{}'.format(round(x,4),round(y,4)))
                   fp3.write('\n')
                   fp3.write('X{}Y{}'.format(round(x+0.1,4),round(y,4)))
                   s += 0.1 
           N += 1
           
   #Long component for X-axis        
       if component_class[index] == 'longX':
           if component_name[index][:4] == 'Free':
               edge_longx = np.zeros(shape=(4,2))
               numb = int(component_name[index].translate({ord(i): None for i in 'Fre'}))
               print(numb)
               edge_longx = globals()['edge_free_0%d' % numb]
               x_max = np.argmax(np.hsplit(globals()['edge_free_0%d' % numb],2)[0])
               y_max = np.argmax(np.hsplit(globals()['edge_free_0%d' % numb],2)[1])
               x_min = np.argmin(np.hsplit(globals()['edge_free_0%d' % numb],2)[0])
               y_min = np.argmin(np.hsplit(globals()['edge_free_0%d' % numb],2)[1])
               x1 = edge_longx[y_min,0] + (edge_longx[max_x,0] - edge_longx[min_y,0])/2
               x2 = edge_longx[x_min,0] + (edge_longx[max_x,0] - edge_longx[min_y,0])/2
               y1 = edge_longx[y_min,1] + (edge_longx[max_x,1] - edge_longx[min_y,1])/2
               y2 = edge_longx[x_min,1] + (edge_longx[max_x,1] - edge_longx[min_y,1])/2
               fp3.write('N{}'.format(N))
               fp3.write('X{}Y{}'.format(round(x1,4),round(y1,4)))
               fp3.write('\n')
               fp3.write('X{}Y{}'.format(round(x2,4),round(y2,4)))   
               fp3.write('\n')
           
           else:                               
               edge_longx[0] = [placement_list[P,0] + (apature_x[index])/2, placement_list[P,1]]
               edge_longx[1] = [placement_list[P,0] - (apature_x[index])/2, placement_list[P,1]]
               while o < len(edge_longx):
                   longx[o] = math.sqrt(math.pow(edge_longx[o,0],2)+math.pow(edge_longx[o,1],2))
                   o += 1
               c = np.argmin(longx)
               v = np.argmax(longx)
               fp3.write('N{}'.format(N))
               fp3.write('X{}Y{}'.format(round(edge_longx[c,0],4),round(edge_longx[c,1],4)))
               fp3.write('\n')
               fp3.write('X{}Y{}'.format(round(edge_longx[v,0],4),round(edge_longx[v,1],4)))   
               fp3.write('\n')
           N += 1

   #Long component for Y-axis    
       if component_class[index] == 'longY':
           if component_name[index][:4] == 'Free':
               edge_longy = np.zeros(shape=(4,2))
               numb = int(component_name[index].translate({ord(i): None for i in 'Fre'}))
               print(numb)
               edge_longy = globals()['edge_free_0%d' % numb]
               x_max = np.argmax(np.hsplit(globals()['edge_free_0%d' % numb],2)[0])
               y_max = np.argmax(np.hsplit(globals()['edge_free_0%d' % numb],2)[1])
               x_min = np.argmin(np.hsplit(globals()['edge_free_0%d' % numb],2)[0])
               y_min = np.argmin(np.hsplit(globals()['edge_free_0%d' % numb],2)[1])
               x1 = edge_longy[y_min,0] + (edge_longx[min_x,0] - edge_longx[min_y,0])/2
               x2 = edge_longy[x_max,0] + (edge_longx[min_x,0] - edge_longx[min_y,0])/2
               y1 = edge_longy[y_min,1] + (edge_longx[min_x,1] - edge_longx[min_y,1])/2
               y2 = edge_longy[x_max,1] + (edge_longx[min_x,1] - edge_longx[min_y,1])/2
               fp3.write('N{}'.format(N))
               fp3.write('X{}Y{}'.format(round(x1,4),round(y1,4)))
               fp3.write('\n')
               fp3.write('X{}Y{}'.format(round(x2,4),round(y2,4)))   
               fp3.write('\n')
               
               
           else:
               edge_longy[0] = [placement_list[P,0], placement_list[P,1] + (apature_y[index])/2]
               edge_longy[1] = [placement_list[P,0], placement_list[P,1] - (apature_y[index])/2]
               while o < len(edge_longy):
                   longy[o] = math.sqrt(math.pow(edge_longy[o,0],2)+math.pow(edge_longy[o,1],2))
                   o += 1
               c = np.argmin(longy)
               v = np.argmax(longy)
               fp3.write('N{}'.format(N))
               fp3.write('X{}Y{}'.format(round(edge_longy[c,0],4),round(edge_longy[c,1],4)))
               fp3.write('\n')
               fp3.write('X{}Y{}'.format(round(edge_longy[v,0],4),round(edge_longy[v,1],4)))
               fp3.write('\n')
           N += 1
               
   
   
   
   #Small component     
       if component_class[index] == 'small': 
           fp3.write('N{}'.format(N))                                                                      
           fp3.write('X{}Y{}'.format(round(placement_list[P,0],4),round(placement_list[P,1],4)))
           fp3.write('\n')
           N += 1
        
   #Deleting past steps
       component_list.pop(P)
       new_place = np.delete(new_place,P,0)
       place = np.delete(place,P,0)
       placement_list = np.delete(placement_list,P,0)
   fp3.close
        