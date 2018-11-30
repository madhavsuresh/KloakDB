import time
import psycopg2
import psycopg2.extensions
from psycopg2.extras import LoggingConnection, LoggingCursor
import logging
import sys
import statistics

logging.basicConfig(level=logging.NOTSET)
logger = logging.getLogger(__name__)

# MyLoggingCursor simply sets self.timestamp at start of each query                                                                 
class MyLoggingCursor(LoggingCursor):
    def execute(self, query, vars=None):
        self.timestamp = time.time()
        return super(MyLoggingCursor, self).execute(query, vars)

    def callproc(self, procname, vars=None):
        self.timestamp = time.time()
        return super(MyLoggingCursor, self).callproc(procname, vars)

# MyLogging Connection:                                                                                                             
#   a) calls MyLoggingCursor rather than the default                                                                                
#   b) adds resulting execution (+ transport) time via filter()                                                                     
class MyLoggingConnection(LoggingConnection):
    def filter(self, msg, curs):
        exec_time = int((time.time() - curs.timestamp) * 1000)
        time_values.append(exec_time)
        return "   %d ms" % exec_time
    def cursor(self, *args, **kwargs):
        kwargs.setdefault('cursor_factory', MyLoggingCursor)
        return LoggingConnection.cursor(self, *args, **kwargs)





conn = psycopg2.connect(connection_factory=MyLoggingConnection, database="healthlnk", user="wdw133", password="vaultdb_rocks")
conn.initialize(logger)
cur = conn.cursor()

def make_comorbidity_query(num,year):
    # 0 is for the current host
    selects = ""
    for i in range(1,num): 
        selects+= "\n UNION ALL\n"
        selects+="select major_icd9 from {0}_cdiff_cohort_diagnoses where year = {1}".format("vdb" + str(i), year)
        
    query = "select d.major_icd9, count(*)\
          FROM( select major_icd9 from cdiff_cohort_diagnoses where year = {1} {0}\
          ) as d\
            group by major_icd9\
            order by count(*) DESC\
            limit 10;".format(selects, year)

    return query




def gen_one_dosage_query(num, year):

    if num == 0:
        machine = ""
    else:
        machine = "vdb" + str(num) + "_"
    return "select d{0}.patient_id \
            from {0}diagnoses d{0}, {0}medications m{0} \
                where d{0}.patient_id=m{0}.patient_id and \
                medication like '%ASPIRIN%' and \
                icd9 like '414%' and \
                dosage='325 MG' and \
                d{0}.year = {1} and m{0}.year = {1}".format(machine, year)

def make_dosage_query(num,year):
    query = ""
    if num_machines == 1:
        query = query_function(0)
    else:
        for i in range(num_machines):
            query += gen_one_dosage_query(i, year)
            if i != num_machines - 1: # don't union on the last time
                query += "\n UNION ALL\n"
    query += ";"
    return query

def make_asprin_query(num,year):
    file = open("aspirin_fdw.sql", "r")
    query = file.read()
    query = query.format(year)
    return query


# query function is a function that takes in a number of machines and returns
# a query 
def run_test(num_machines, num_runs,query_function, year):
    query = query_function(num_machines, year)
    for i in range(num_runs):
        c = cur.execute(query)
    time_values.pop(0) # remove the first one b/c takes a long time
    avg = statistics.mean(time_values)
    standard_dev = statistics.stdev(time_values)
    print("removed first timed run")
    print("the average is: " + str(avg))
    print("the stdev is: " + str(standard_dev))

num_machines = int(sys.argv[1])
num_runs = int(sys.argv[2])
search_year = str(sys.argv[3])
if num_machines < 1 or num_machines > 4:
    print("MUST RUN ON AT LEAST ONE MACHINE AND NOT MORE THAN 4")
    sys.exit()
if num_runs < 3:
    print("must run atleast 3 runs")
    sys.exit()
print("ASPIRIN TEST")
time_values = []
run_test(num_machines, num_runs, make_asprin_query, search_year)

print("COMORBIDITY TEST")
time_values = []
run_test(num_machines,num_runs, make_comorbidity_query, search_year)

print("DOSAGE TEST")
time_values = []
run_test(num_machines, num_runs, make_dosage_query, search_year)


