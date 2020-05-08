# -*- coding: utf-8 -*-
"""
Created on Tue May  5 14:18:57 2020

@author: fredr
"""

#Importing libraries
import numpy as np
import math

#Filepaths
#filepath1 = 'SwitchingSupply-Edge_Cuts.gbr'
#filepath2 = 'SwitchingSupply-F_Paste.gbr'
#filepath3 = 'testskrive.txt'
def create_G_file(filepath1, filepath2, filepath3):
    #Declering variabels
    k = 0
    i = 0
    n = 0
    m = 0
    j = 0
    h = 0
    start_placement = 0

    #Declering arrays & lists
    edge_cut = []
    apature_list = []
    placement_list = []
    component_list = []
    apature = []
    component_sizes = []
    component_class = []
    current_posision = np.zeros(shape=(1,2))

    #Reading the edge cut file and storing data
    with open (filepath1) as fp1:
        lines = fp1.readlines()
        while k < len(lines):
            string = lines[k]
            if string[:1] == 'X':
                edge_cut.append(string)
            k += 1

    #Reading the paste file and storing data
    with open (filepath2) as fp2:
        lines = fp2.readlines()
        while n < len(lines):
            string = lines[n]
        
            #Finding the apatures for the PCB
            if '%ADD' in string:
                if 'R'in string:
                    shape = ('R')
                if 'S' in string:
                    shape = ('S')
                apature_list.append(string)
        
            #Finding apature names
            if string[:1] == 'D':
                D = string.translate({ord(i): None for i in '*'})
                start_placement = 1
        
            #End of document
            if string[:1] == 'M':
                start_placement = 0
        
            #Finding the apature placements
            if start_placement == 1:
                if string[:1] != 'D':
                    placement_list.append(string)
                    component_list.append(D)
            n +=1
        

    #Converting data to the right format
    while i < len(edge_cut):
        edge_cut[i] = edge_cut[i].split('D')
        edge_cut[i] = edge_cut[i][0]
        edge_cut[i] = edge_cut[i].translate({ord(i): None for i in 'X'})
        edge_cut[i] = edge_cut[i].split('Y')
        edge_cut[i] = [int(edge_cut[i][0]), int(edge_cut[i][1])]
        i += 1
    
    while j < len(apature_list):
        apature_list[j] = apature_list[j].translate({ord(i): None for i in '%AD*'})
        if 'R' in apature_list[j]:
            apature_list[j] = apature_list[j].split('R,')
        if 'S' in apature_list[j]:
            apature_list[j] = apature_list[j].split('S,')
        apature.append('D{}'.format(apature_list[j][0]))
        apature_list[j] = apature_list[j][1].split('X')
        apature_list[j] = ([float(apature_list[j][0]),float(apature_list[j][1])])
        j += 1

    while m < len(placement_list):
        placement_list[m] = placement_list[m].split('D')
        placement_list[m] = placement_list[m][0]
        placement_list[m] = placement_list[m].translate({ord(i): None for i in 'X'})
        placement_list[m] = placement_list[m].split('Y')
        placement_list[m] = [float(placement_list[m][0]),float(placement_list[m][1])]
        m += 1
  
    #Finding the offset from the edge cut data
    edge_cut = np.array(edge_cut)
    offset_x = np.argmax(np.hsplit(edge_cut,2)[0])
    offset_y = np.argmin(np.hsplit(edge_cut,2)[1])
    offset = np.array([edge_cut[offset_x][0],edge_cut[offset_y][1]])/1000000

    #Calculating the area of the components
    apature_list = np.array(apature_list)
    apature_x = np.hsplit(apature_list,2)[0]
    apature_y = np.hsplit(apature_list,2)[1]
    apature = apature_x * apature_y

    #Comansating for the offset in gerber file
    placement_list = np.array(placement_list)/1000000
    placement_list = placement_list - offset

    #Flipping the X-axis
    placement_list[:,0] = placement_list[:,0] *-1

    #Assigning class to the components
    for rows in apature:
        if apature[h] < 1:
            component_class.append('big')
        elif apature_y[h] < apature_x[h]*2:
            component_class.append('longY')
        elif apature_x[h] < apature_y[h]*2:
            component_class.append('longX')
        else:
            component_class.append('small')
        h += 1
    
    #Saving important arrays and variabels
    place = placement_list
    component_name = list(dict.fromkeys(component_list))

    #Calculating the path trough the PCB
    fp3 = open(filepath3,'w+')
    fp3.write('G0')
    fp3.write('\n')
    for rows in place:
        place = place - current_posision
        new_place = np.empty(len(place))
        l = 0
        #Finding next step for the path
        for rows in new_place:
            new_place[l] = math.sqrt(math.pow(place[l,0],2)+math.pow(place[l,1],2))
            l += 1
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
            edge_big[0] = [placement_list[P,0] - (apature_x[index])/2, placement_list[P,1] - (apature_y[index])/2]
            edge_big[1] = [placement_list[P,0] - (apature_x[index])/2, placement_list[P,1] + (apature_y[index])/2]
            edge_big[2] = [placement_list[P,0] + (apature_x[index])/2, placement_list[P,1] + (apature_y[index])/2]
            edge_big[3] = [placement_list[P,0] + (apature_x[index])/2, placement_list[P,1] - (apature_y[index])/2]
            while o < len(edge_big):
                big[o] = math.sqrt(math.pow(edge_big[o,0],2)+math.pow(edge_big[o,1],2))
                o += 1
            c = np.argmin(big)
            toggle = True
        
            if c == 0:
                while s > apature_x[index]:
                    x = edge_big[0,0] + s
                    if toggle == True:
                        y = edge_big[0,1]
                        toggle = not toggle
                    if toggle == False:
                        y = edge_big[1,1]
                        toggle = not toggle
                    fp3.write('X{}Y{}big0'.format(x,y))
                    fp3.write('\n')
                    s =+ 1
                
            if c == 1:
                while s > apature_x[index]:
                    x = edge_big[0,0] + s
                    if toggle == True:
                        y = edge_big[1,1]
                        toggle = not toggle
                    if toggle == False:
                        y = edge_big[0,1]
                        toggle = not toggle
                    fp3.write('X{}Y{}big1'.format(x,y))
                    fp3.write('\n')
                    s =+ 1
                
            if c == 2:
                while s > apature_x[index]:
                    x = edge_big[0,0] - s
                    if toggle == True:
                        y = edge_big[1,1]
                        toggle = not toggle
                    if toggle == False:
                        y = edge_big[0,1]
                        toggle = not toggle
                    fp3.write('X{}Y{}big2'.format(x,y))
                    fp3.write('\n')
                    s =+ 1
                
            if c == 3:
                while s > apature_x[index]:
                    x = edge_big[0,0] - s
                    if toggle == True:
                        y = edge_big[0,1]
                        toggle = not toggle
                    if toggle == False:
                        y = edge_big[1,1]
                        toggle = not toggle
                    fp3.write('X{}Y{}big3'.format(x,y))
                    fp3.write('\n')
                    s =+ 1
                
        #Long component for X-axis
        if component_class[index] == 'longX':
            edge_longx[0] = [placement_list[P,0] + (apature_x[index])/2, placement_list[P,1]]
            edge_longx[1] = [placement_list[P,0] - (apature_x[index])/2, placement_list[P,1]]
            while o < len(edge_longx):
                longx[o] = math.sqrt(math.pow(edge_longx[o,0],2)+math.pow(edge_longx[o,1],2))
                o += 1
            c = np.argmin(longx)
            v = np.argmax(longx)
            fp3.write('X{}Y{}longx'.format(edge_longx[c,0],edge_longx[c,1]))
            fp3.write('\n')
            fp3.write('X{}Y{}'.format(edge_longx[v,0],edge_longx[v,1]))
            fp3.write('\n')
        
        #Long component for Y-axis
        if component_class[index] == 'longY':
            edge_longy[0] = [placement_list[P,0], placement_list[P,1] + (apature_y[index])/2]
            edge_longy[1] = [placement_list[P,0], placement_list[P,1] - (apature_y[index])/2]
            while o < len(edge_longy):
                longy[o] = math.sqrt(math.pow(edge_longy[o,0],2)+math.pow(edge_longy[o,1],2))
                o += 1
            c = np.argmin(longy)
            v = np.argmax(longy)
            fp3.write('X{}Y{}longy'.format(edge_longy[c,0],edge_longy[c,1]))
            fp3.write('\n')
            fp3.write('X{}Y{}'.format(edge_longy[v,0],edge_longy[v,1]))
            fp3.write('\n')
    
        #Small component
        else:
            fp3.write('X{}Y{}small'.format(placement_list[P,0],placement_list[P,1]))
            fp3.write('\n')
        
        #Deleting past steps
        component_list.pop(P)
        new_place = np.delete(new_place,P,0)
        place = np.delete(place,P,0)
        placement_list = np.delete(placement_list,P,0)
    fp3.close




















