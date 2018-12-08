select setseed(.0002);
DROP TABLE IF EXISTS dem_250, diag_250, vitals_250, medications_250;

SELECT * INTO dem_250 FROM (SELECT * FROM demographics ORDER BY random()) t LIMIT 250;
SELECT * INTO diag_250 FROM (SELECT * FROM diagnoses ORDER BY random()) t LIMIT 250;
SELECT * INTO vitals_250 FROM (SELECT * FROM vitals WHERE pulse is NOT NULL ORDER BY random()) t LIMIT 250;
SELECT * INTO medications_250 FROM (SELECT * FROM medications ORDER BY random()) t LIMIT 250;

DROP TABLE IF EXISTS dem_125, diag_125, vitals_125, medications_125;

SELECT * INTO dem_125 FROM (SELECT * FROM demographics ORDER BY random()) t LIMIT 125;
SELECT * INTO diag_125 FROM (SELECT * FROM diagnoses ORDER BY random()) t LIMIT 125;
SELECT * INTO vitals_125 FROM (SELECT * FROM vitals WHERE pulse is NOT NULL ORDER BY random()) t LIMIT 125;
SELECT * INTO medications_125 FROM (SELECT * FROM medications ORDER BY random()) t LIMIT 125;

DROP TABLE IF EXISTS dem_ex, diag_ex, vit_ex, meds_ex;
SELECT * INTO dem_ex FROM (SELECT * FROM demographics)
SELECT * INTO diag_ex FROM (SELECT * FROM diagnoses WHERE year=2008);
SELECT * INTO vit_ex FROM (SELECT * FROM vitals WHERE year=2008 AND pulse IS NOT NULL);
SELECT * INTO meds_ex FROM (SELECT * FROM medications WHERE year=2008);
