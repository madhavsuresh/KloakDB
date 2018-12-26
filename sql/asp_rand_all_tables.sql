DROP TABLE IF EXISTS dem_ex_asp_1, vit_ex_asp_1, meds_ex_asp_1;
SELECT * INTO dem_ex_asp_1 FROM (SELECT * FROM dem_ex where patient_id in (SELECT * from hd_cohort_dist_25_1)) t;
SELECT * INTO vit_ex_asp_1 FROM (SELECT * FROM vit_ex where patient_id in (SELECT * from hd_cohort_dist_25_1)) t;
SELECT * INTO meds_ex_asp_1 FROM (SELECT * FROM meds_ex where patient_id in (SELECT * from hd_cohort_dist_25_1)) t;

DROP TABLE IF EXISTS dem_ex_asp_2, vit_ex_asp_2, meds_ex_asp_2;
SELECT * INTO dem_ex_asp_2 FROM (SELECT * FROM dem_ex where patient_id in (SELECT * from hd_cohort_dist_25_2)) t;
SELECT * INTO vit_ex_asp_2 FROM (SELECT * FROM vit_ex where patient_id in (SELECT * from hd_cohort_dist_25_2)) t;
SELECT * INTO meds_ex_asp_2 FROM (SELECT * FROM meds_ex where patient_id in (SELECT * from hd_cohort_dist_25_2)) t;

DROP TABLE IF EXISTS dem_ex_asp_3, vit_ex_asp_3, meds_ex_asp_3;
SELECT * INTO dem_ex_asp_3 FROM (SELECT * FROM dem_ex where patient_id in (SELECT * from hd_cohort_dist_25_3)) t;
SELECT * INTO vit_ex_asp_3 FROM (SELECT * FROM vit_ex where patient_id in (SELECT * from hd_cohort_dist_25_3)) t;
SELECT * INTO meds_ex_asp_3 FROM (SELECT * FROM meds_ex where patient_id in (SELECT * from hd_cohort_dist_25_3)) t;

DROP TABLE IF EXISTS dem_ex_asp_4, vit_ex_asp_4, meds_ex_asp_4;
SELECT * INTO dem_ex_asp_4 FROM (SELECT * FROM dem_ex where patient_id in (SELECT * from hd_cohort_dist_25_4)) t;
SELECT * INTO vit_ex_asp_4 FROM (SELECT * FROM vit_ex where patient_id in (SELECT * from hd_cohort_dist_25_4)) t;
SELECT * INTO meds_ex_asp_4 FROM (SELECT * FROM meds_ex where patient_id in (SELECT * from hd_cohort_dist_25_4)) t;

DROP TABLE IF EXISTS dem_ex_asp_5, vit_ex_asp_5, meds_ex_asp_5;
SELECT * INTO dem_ex_asp_5 FROM (SELECT * FROM dem_ex where patient_id in (SELECT * from hd_cohort_dist_25_5)) t;
SELECT * INTO vit_ex_asp_5 FROM (SELECT * FROM vit_ex where patient_id in (SELECT * from hd_cohort_dist_25_5)) t;
SELECT * INTO meds_ex_asp_5 FROM (SELECT * FROM meds_ex where patient_id in (SELECT * from hd_cohort_dist_25_5)) t;

DROP TABLE IF EXISTS dem_ex_asp_6, vit_ex_asp_6, meds_ex_asp_6;
SELECT * INTO dem_ex_asp_6 FROM (SELECT * FROM dem_ex where patient_id in (SELECT * from hd_cohort_dist_25_6)) t;
SELECT * INTO vit_ex_asp_6 FROM (SELECT * FROM vit_ex where patient_id in (SELECT * from hd_cohort_dist_25_6)) t;
SELECT * INTO meds_ex_asp_6 FROM (SELECT * FROM meds_ex where patient_id in (SELECT * from hd_cohort_dist_25_6)) t;

DROP TABLE IF EXISTS dem_ex_asp_7, vit_ex_asp_7, meds_ex_asp_7;
SELECT * INTO dem_ex_asp_7 FROM (SELECT * FROM dem_ex where patient_id in (SELECT * from hd_cohort_dist_25_7)) t;
SELECT * INTO vit_ex_asp_7 FROM (SELECT * FROM vit_ex where patient_id in (SELECT * from hd_cohort_dist_25_7)) t;
SELECT * INTO meds_ex_asp_7 FROM (SELECT * FROM meds_ex where patient_id in (SELECT * from hd_cohort_dist_25_7)) t;
