import statistics
import sys
file_name = 'exp5_logjam.log'
exp5_dict = {}
f = open(file_name, 'r')
for line in f:
    if 'EXP5-STAT' in line:
        if 'join' not in line:
            continue
        out = line.split('|')
        if len(out) > 1:
            stat = out[1].split(',')
            p_stat = float(stat[2].strip().strip('""'))
            if stat[1] in exp5_dict:
                if stat[0] in exp5_dict[stat[1]]:
                    exp5_dict[stat[1]][stat[0]].append(p_stat)
                else:
                    exp5_dict[stat[1]][stat[0]] = [p_stat,]
            else:
                exp5_dict[stat[1]] = {}
                exp5_dict[stat[1]][stat[0]] = [p_stat,]
                

for i in exp5_dict:
    print i , "- ANON"
    for j in exp5_dict[i]:
        print j,  ":",  statistics.mean(exp5_dict[i][j])
