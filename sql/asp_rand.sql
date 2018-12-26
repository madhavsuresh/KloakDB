---RANDOM 25 for ASPIRIN Profile Oblivious Queries
select setseed(.0001);
DROP TABLE IF EXISTS hd_cohort_dist_25_1;
SELECT * INTO hd_cohort_dist_25_1 FROM (SELECT * FROM hd_cohort_dist ORDER BY random()) t LIMIT 25;

DROP TABLE IF EXISTS hd_cohort_dist_25_2;
SELECT * INTO hd_cohort_dist_25_2 FROM (SELECT * FROM hd_cohort_dist ORDER BY random()) t LIMIT 25;

DROP TABLE IF EXISTS hd_cohort_dist_25_3;
SELECT * INTO hd_cohort_dist_25_3 FROM (SELECT * FROM hd_cohort_dist ORDER BY random()) t LIMIT 25;

DROP TABLE IF EXISTS hd_cohort_dist_25_4;
SELECT * INTO hd_cohort_dist_25_4 FROM (SELECT * FROM hd_cohort_dist ORDER BY random()) t LIMIT 25;

DROP TABLE IF EXISTS hd_cohort_dist_25_5;
SELECT * INTO hd_cohort_dist_25_5 FROM (SELECT * FROM hd_cohort_dist ORDER BY random()) t LIMIT 25;

DROP TABLE IF EXISTS hd_cohort_dist_25_6;
SELECT * INTO hd_cohort_dist_25_6 FROM (SELECT * FROM hd_cohort_dist ORDER BY random()) t LIMIT 25;

DROP TABLE IF EXISTS hd_cohort_dist_25_7;
SELECT * INTO hd_cohort_dist_25_7 FROM (SELECT * FROM hd_cohort_dist ORDER BY random()) t LIMIT 25;

DROP TABLE IF EXISTS hd_cohort_dist_25_8;
SELECT * INTO hd_cohort_dist_25_8 FROM (SELECT * FROM hd_cohort_dist ORDER BY random()) t LIMIT 25;

DROP TABLE IF EXISTS hd_cohort_dist_25_9;
SELECT * INTO hd_cohort_dist_25_9 FROM (SELECT * FROM hd_cohort_dist ORDER BY random()) t LIMIT 25;

DROP TABLE IF EXISTS hd_cohort_dist_25_10;
SELECT * INTO hd_cohort_dist_25_10 FROM (SELECT * FROM hd_cohort_dist ORDER BY random()) t LIMIT 25;
