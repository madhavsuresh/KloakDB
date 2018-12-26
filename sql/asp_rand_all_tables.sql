DROP TABLE IF EXISTS dem_ex_asp_1, vit_ex_asp_1, meds_ex_asp_1;
SELECT * INTO dem_ex_asp_1 FROM (SELECT * FROM dem_ex where patient_id in (SELECT * from hd_cohort_dist_25_1)) t;
SELECT * INTO vit_ex_asp_1 FROM (SELECT * FROM vit_ex where patient_id in (SELECT * from hd_cohort_dist_25_1)) t;
SELECT * INTO meds_ex_asp_1 FROM (SELECT * FROM meds_ex where patient_id in (SELECT * from hd_cohort_dist_25_1)) t;
