select setseed(.0001);
DROP TABLE IF EXISTS dem_250, diag_250, vitals_250, medications_250;

SELECT * INTO dem_250 FROM (SELECT * FROM demographics ORDER BY random()) t LIMIT 250;
SELECT * INTO diag_250 FROM (SELECT * FROM diagnoses ORDER BY random()) t LIMIT 250;
SELECT * INTO vitals_250 FROM (SELECT * FROM vitals ORDER BY random()) t LIMIT 250;
SELECT * INTO medications_250 FROM (SELECT * FROM medications ORDER BY random()) t LIMIT 250;
