DROP TABLE IF EXISTS dem_ex, diag_ex, vit_ex, meds_ex, hd_cohort;
SELECT * INTO hd_cohort FROM (SELECT DISTINCT patient_id from diagnoses WHERE icd9 LIKE '414%') t;
SELECT * INTO dem_ex FROM (SELECT * FROM demographics where patient_id in (SELECT * from hd_cohort)) t;
SELECT * INTO vit_ex FROM (SELECT * FROM vitals WHERE pulse IS NOT NULL AND patient_id in (SELECT * from hd_cohort)) t;
SELECT * INTO meds_ex FROM (SELECT * FROM medications WHERE patient_id in (SELECT * from hd_cohort)) t; ---AND medication ILIKE '%aspirin%') t;
