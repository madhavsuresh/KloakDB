DROP TABLE IF EXISTS dem_ex, diag_ex, vit_ex, meds_ex, hd_cohort;
SELECT * INTO hd_cohort FROM (SELECT DISTINCT patient_id from diagnoses WHERE icd9 LIKE '414%' AND (year=2008)) t;
SELECT * INTO dem_ex FROM (SELECT * FROM demographics where patient_id in (SELECT * from hd_cohort_dist)) t;
SELECT * INTO vit_ex FROM (SELECT * FROM vitals WHERE pulse IS NOT NULL AND (year=2008) AND patient_id in (SELECT * from hd_cohort_dist)) t;
SELECT * INTO meds_ex FROM (SELECT * FROM medications WHERE (year=2008) AND patient_id in (SELECT * from hd_cohort_dist)) t; ---AND medication ILIKE '%aspirin%') t;

--- LOCAL, used for semi-join optimization
DROP TABLE IF EXISTS dem_ex_local, diag_ex_local, vit_ex_local, meds_ex_local, hd_cohort_local;
SELECT * into hd_cohort_local FROM (SELECT patient_id FROM hd_cohort WHERE patient_id NOT IN (SELECT * FROM hd_cohort_dist)) t;
SELECT * INTO dem_ex_local FROM (SELECT * FROM demographics where patient_id in (SELECT * from hd_cohort_local)) t;
SELECT * INTO vit_ex_local FROM (SELECT * FROM vitals WHERE pulse IS NOT NULL AND (year=2008) AND patient_id in (SELECT * from hd_cohort_local)) t;
SELECT * INTO meds_ex_local FROM (SELECT * FROM medications WHERE (year=2008) AND patient_id in (SELECT * from hd_cohort_local)) t; ---AND medication ILIKE '%aspirin%') t;

-- FDW, used for FDW queries
DROP TABLE IF EXISTS dem_ex_fdw, vit_ex_fdw, meds_ex_fdw;
SELECT * INTO dem_ex_fdw FROM (SELECT * FROM demographics where patient_id in (SELECT * from hd_cohort)) t;
SELECT * INTO vit_ex_fdw FROM (SELECT * FROM vitals WHERE pulse IS NOT NULL AND (year=2008) AND patient_id in (SELECT * from hd_cohort)) t;
SELECT * INTO meds_ex_fdw FROM (SELECT * FROM medications WHERE (year=2008) AND patient_id in (SELECT * from hd_cohort)) t; ---AND medication ILIKE '%aspirin%') t;
