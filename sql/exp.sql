DROP TABLE IF EXISTS dem_ex, diag_ex, vit_ex, meds_ex;
SELECT * INTO dem_ex FROM (SELECT * FROM demographics) t;
SELECT * INTO diag_ex FROM (SELECT * FROM diagnoses WHERE year=2008 AND icd9 LIKE '414%') t;
SELECT * INTO vit_ex FROM (SELECT * FROM vitals WHERE year=2008 AND pulse IS NOT NULL) t;
SELECT * INTO meds_ex FROM (SELECT * FROM medications WHERE year=2008 AND medication ILIKE '%aspirin%') t;
