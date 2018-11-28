import statistics
f = open('exp3_10000_logjam.log', 'r')
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
for i in stats_dict:
    print i, statistics.mean(stats_dict[i])*1000 , " stddev", statistics.stdev(stats_dict[i]), " median:", statistics.median(stats_dict[i]) 

accumulator = 0.0
for i in stats_dict:
    if 'data_movement' in i:
        accumlator = accumulator + statistics.mean(stats_dict[i])
print "RPC time: ", accumlator*1000

for i in stats_dict:
    if 'outer' in i:
        print "num samples:", len(stats_dict[i])
        break
                
