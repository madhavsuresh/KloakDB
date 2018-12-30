DROP TABLE IF EXISTS meds_ex_dos_100;
SELECT * INTO meds_ex_dos_100 FROM (SELECT * FROM medications where patient_id in (SELECT * from hd_cohort_dist_all_100)) t;

DROP TABLE IF EXISTS meds_ex_dos_500;
SELECT * INTO meds_ex_dos_500 FROM (SELECT * FROM medications where patient_id in (SELECT * from hd_cohort_dist_all_500)) t;

DROP TABLE IF EXISTS meds_ex_dos_1000;
SELECT * INTO meds_ex_dos_1000 FROM (SELECT * FROM medications where patient_id in (SELECT * from hd_cohort_dist_all_1000)) t;

DROP TABLE IF EXISTS meds_ex_dos_2000;
SELECT * INTO meds_ex_dos_2000 FROM (SELECT * FROM medications where patient_id in (SELECT * from hd_cohort_dist_all_2000)) t;

DROP TABLE IF EXISTS meds_ex_dos_3000;
SELECT * INTO meds_ex_dos_3000 FROM (SELECT * FROM medications where patient_id in (SELECT * from hd_cohort_dist_all_3000)) t;

DROP TABLE IF EXISTS meds_ex_dos_4000;
SELECT * INTO meds_ex_dos_4000 FROM (SELECT * FROM medications where patient_id in (SELECT * from hd_cohort_dist_all_4000)) t;

DROP TABLE IF EXISTS meds_ex_dos_5000;
SELECT * INTO meds_ex_dos_5000 FROM (SELECT * FROM medications where patient_id in (SELECT * from hd_cohort_dist_all_5000)) t;

