---RANDOM samples for dosage study scale up
select setseed(.0001);
DROP TABLE IF EXISTS hd_cohort_dist_all_100;
SELECT * INTO hd_cohort_dist_all_100 FROM (SELECT * FROM hd_cohort_dist_all ORDER BY random()) t LIMIT 100;

DROP TABLE IF EXISTS hd_cohort_dist_all_500;
SELECT * INTO hd_cohort_dist_all_500 FROM (SELECT * FROM hd_cohort_dist_all ORDER BY random()) t LIMIT 500;

DROP TABLE IF EXISTS hd_cohort_dist_all_1000;
SELECT * INTO hd_cohort_dist_all_1000 FROM (SELECT * FROM hd_cohort_dist_all ORDER BY random()) t LIMIT 1000;

DROP TABLE IF EXISTS hd_cohort_dist_all_2000;
SELECT * INTO hd_cohort_dist_all_2000 FROM (SELECT * FROM hd_cohort_dist_all ORDER BY random()) t LIMIT 2000;

DROP TABLE IF EXISTS hd_cohort_dist_all_5000;
SELECT * INTO hd_cohort_dist_all_5000 FROM (SELECT * FROM hd_cohort_dist_all ORDER BY random()) t LIMIT 5000;
