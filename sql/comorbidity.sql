DROP TABLE IF EXISTS cdiff_cohort_diagnoses;
CREATE TABLE cdiff_cohort_diagnoses AS SELECT * FROM diagnoses WHERE patient_id IN (SELECT * FROM cdiff_cohort); 

SELECT d.major_icd9, count(*) as cnt 
 FROM cdiff_cohort_diagnoses d                                                                                         GROUP BY d.major_icd9                                                        
 ORDER BY count(*) DESC 
 LIMIT 10;

