DROP TABLE IF EXISTS dem_ex, diag_ex, vit_ex, meds_ex, hd_cohort;
SELECT * INTO hd_cohort FROM (SELECT DISTINCT patient_id from diagnoses WHERE icd9 LIKE '414%' AND year=2008) t;
SELECT * INTO dem_ex FROM (SELECT * FROM demographics where patient_id in (SELECT * from hd_cohort_dist)) t;
SELECT * INTO vit_ex FROM (SELECT * FROM vitals WHERE pulse IS NOT NULL AND year=2008 AND patient_id in (SELECT * from hd_cohort_dist)) t;
SELECT * INTO meds_ex FROM (SELECT * FROM medications WHERE year=2008 AND patient_id in (SELECT * from hd_cohort_dist)) t; ---AND medication ILIKE '%aspirin%') t;
