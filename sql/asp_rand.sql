---RANDOM 25 for ASPIRIN Profile Oblivious Queries
select setseed(.0001);
SELECT * INTO hd_cohort_dist_25_1 FROM (SELECT * FROM hd_cohort_dist ORDER BY random()) t LIMIT 25;
SELECT * INTO meds_ex_25_1 FROM (SELECT * FROM meds_ex ORDER BY random()) t LIMIT 25;
SELECT * INTO vit_ex_25_1 FROM (SELECT * FROM vit_ex ORDER BY random()) t LIMIT 25;
SELECT * INTO dem_ex_25_1 FROM (SELECT * FROM dem_ex ORDER BY random()) t LIMIT 25;

SELECT * INTO hd_cohort_dist_25_2 FROM (SELECT * FROM hd_cohort_dist ORDER BY random()) t LIMIT 25;
SELECT * INTO meds_ex_25_2 FROM (SELECT * FROM meds_ex ORDER BY random()) t LIMIT 25;
SELECT * INTO vit_ex_25_2 FROM (SELECT * FROM vit_ex ORDER BY random()) t LIMIT 25;
SELECT * INTO dem_ex_25_2 FROM (SELECT * FROM dem_ex ORDER BY random()) t LIMIT 25;

SELECT * INTO hd_cohort_dist_25_3 FROM (SELECT * FROM hd_cohort_dist ORDER BY random()) t LIMIT 25;
SELECT * INTO meds_ex_25_3 FROM (SELECT * FROM meds_ex ORDER BY random()) t LIMIT 25;
SELECT * INTO vit_ex_25_3 FROM (SELECT * FROM vit_ex ORDER BY random()) t LIMIT 25;
SELECT * INTO dem_ex_25_3 FROM (SELECT * FROM dem_ex ORDER BY random()) t LIMIT 25;

SELECT * INTO hd_cohort_dist_25_4 FROM (SELECT * FROM hd_cohort_dist ORDER BY random()) t LIMIT 25;
SELECT * INTO meds_ex_25_4 FROM (SELECT * FROM meds_ex ORDER BY random()) t LIMIT 25;
SELECT * INTO vit_ex_25_4 FROM (SELECT * FROM vit_ex ORDER BY random()) t LIMIT 25;
SELECT * INTO dem_ex_25_4 FROM (SELECT * FROM dem_ex ORDER BY random()) t LIMIT 25;

SELECT * INTO hd_cohort_dist_25_5 FROM (SELECT * FROM hd_cohort_dist ORDER BY random()) t LIMIT 25;
SELECT * INTO meds_ex_25_5 FROM (SELECT * FROM meds_ex ORDER BY random()) t LIMIT 25;
SELECT * INTO vit_ex_25_5 FROM (SELECT * FROM vit_ex ORDER BY random()) t LIMIT 25;
SELECT * INTO dem_ex_25_5 FROM (SELECT * FROM dem_ex ORDER BY random()) t LIMIT 25;

SELECT * INTO hd_cohort_dist_25_6 FROM (SELECT * FROM hd_cohort_dist ORDER BY random()) t LIMIT 25;
SELECT * INTO meds_ex_25_6 FROM (SELECT * FROM meds_ex ORDER BY random()) t LIMIT 25;
SELECT * INTO vit_ex_25_6 FROM (SELECT * FROM vit_ex ORDER BY random()) t LIMIT 25;
SELECT * INTO dem_ex_25_6 FROM (SELECT * FROM dem_ex ORDER BY random()) t LIMIT 25;

SELECT * INTO hd_cohort_dist_25_7 FROM (SELECT * FROM hd_cohort_dist ORDER BY random()) t LIMIT 25;
SELECT * INTO meds_ex_25_7 FROM (SELECT * FROM meds_ex ORDER BY random()) t LIMIT 25;
SELECT * INTO vit_ex_25_7 FROM (SELECT * FROM vit_ex ORDER BY random()) t LIMIT 25;
SELECT * INTO dem_ex_25_7 FROM (SELECT * FROM dem_ex ORDER BY random()) t LIMIT 25;
