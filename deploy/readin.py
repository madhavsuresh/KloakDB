import statistics
import sys
file_name = 'exp3_' + sys.argv[1] + '_logjam.log'
f = open(file_name, 'r')
stats_dict = {}
for line in f:
    if 'EXP3-STAT' in line:
        out = line.split('|')
        if len(out) > 1:
            stat = out[1].split(',')
            p_stat = float(stat[1].strip().strip('""'))
            if stat[0] in stats_dict:
                stats_dict[stat[0]].append(p_stat)
            else:
                stats_dict[stat[0]] = [p_stat,]

accumulator = 0.0
for i in stats_dict:
    if 'data_movement' in i:
        accumlator = accumulator + statistics.mean(stats_dict[i])
#print "RPC time: ", accumlator*1000

for i in stats_dict:
    if 'outer' in i:
        print "outer num samples:", len(stats_dict[i])
        break
                
oh = statistics.mean(stats_dict['repartition_step_one_outer_private']) + statistics.mean(stats_dict['repartition_step_two_outer_private']) - statistics.mean(stats_dict['repart_one_shuffle_assign']) - statistics.mean(stats_dict['repart_two_hashing']) + statistics.mean(stats_dict['repartition_coalesce_outer_private'])
stats_dict['oh'] = oh

cared = ['repart_one_shuffle_assign', 'repart_two_hashing', 'oh', 'repartition_outer']
for i in cared:
    if i == 'oh':
        print 'Overhead: ' , stats_dict['oh']*1000
    else:
        print i, statistics.mean(stats_dict[i])*1000 #, " stddev", statistics.stdev(stats_dict[i]), " median:", statistics.median(stats_dict[i]) 

