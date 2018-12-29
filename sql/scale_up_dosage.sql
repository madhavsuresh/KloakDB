DROP TABLE IF EXISTS hd_cohort_all;
SELECT * INTO hd_cohort_all FROM (SELECT DISTINCT patient_id from diagnoses WHERE icd9 LIKE '414%') t;
