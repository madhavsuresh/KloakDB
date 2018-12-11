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
    year_filter = ""
    if year == None:
        year_filter = ""
    else: 
        year_filter = "where year = {0}".format(year)
    for i in range(1,num+1): 
        selects+="select major_icd9 from {0}.cdiff_cohort_diagnoses {1} ".format("vdb" + str(i), year_filter)
        if i != num:
            selects+= "\n UNION ALL\n"
        
    query = "select d.major_icd9, count(*)\
          FROM({0}\
          ) as d\
            group by major_icd9\
            order by count(*) DESC\
            limit 10;".format(selects, year_filter)

    return query


def gen_union_dosage(num,year):
    med_tables = ""
    diag_tables = ""
    if year == None:
        year_filter = ""
    else: 
        year_filter = "and year = {0}".format(year)

    for i in range(1,num+1):
        med_tables += "SELECT patient_id FROM {0}.medications where medication like '%ASPIRIN' and dosage='325 MG' {1}".format("vdb"+str(i),year_filter)
        diag_tables += "SELECT patient_id FROM {0}.diagnoses where icd9 like '414%' {1}".format("vdb"+str(i), year_filter)
        if i != num:
            med_tables += " UNION ALL "
            diag_tables += " UNION ALL "
    return med_tables,diag_tables

def make_dosage_query(num,year):
    m_tables,d_tables = gen_union_dosage(num,year)
    query = "WITH medications_all AS ({0}),\
            diagnoses_all AS ({1})\
            SELECT m.patient_id FROM medications_all m, diagnoses_all di WHERE \
                    m.patient_id = di.patient_id;".format(m_tables,d_tables)
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
    time_values.remove(max(time_values))
    time_values.remove(min(time_values))
    avg = statistics.mean(time_values)
    standard_dev = statistics.stdev(time_values)
    print("removed longest and shortest timed runs")
    print("the average is: " + str(avg))
    print("the stdev is: " + str(standard_dev))

import argparse

parser=argparse.ArgumentParser()
parser.add_argument('-num_machines', help='number of machines you want the query to run on. Must be between 1 and 4. Default is 4')
parser.add_argument('-num_runs', help='number of times you want to run the query. Must be be at least 4 for accurate statistics. Default is 4')
parser.add_argument('-search_year', help='year to filter query on, if blank no filtering')
parser.add_argument('-query', help='aspirin, comorbidity, or dosage. Default runs all queries') 

args = parser.parse_args()

all_queries = ['comorbidity', 'dosage', 'aspirin']

num_machines = 4 if args.num_machines == None else int(args.num_machines)
num_runs = 4 if args.num_runs == None else int(args.num_runs)
search_year = args.search_year
query = all_queries if args.query == None else [args.query]
if num_machines < 1 or num_machines > 4:
    print("MUST RUN ON AT LEAST ONE MACHINE AND NOT MORE THAN 4")
    sys.exit()
if num_runs < 4:
    print("must run atleast 4 runs (for std dev)")
    sys.exit()

for test in query:
    if test == 'aspirin':
        print("ASPIRIN TEST")
        time_values = []
        run_test(num_machines, num_runs, make_asprin_query, search_year)
    elif test == 'comorbidity':
        print("COMORBIDITY TEST")
        time_values = []
        run_test(num_machines,num_runs, make_comorbidity_query, search_year)

    elif test == 'dosage':
        print("DOSAGE TEST")
        time_values = []
        run_test(num_machines, num_runs, make_dosage_query, search_year)

    else:
        print("You passed an incorrect value to query")

