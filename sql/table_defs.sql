-- experimental config
CREATE TABLE demographics (
       patient_id integer,
       birth_year integer,
       gender integer,
       race integer,
       ethnicity integer,
       insurance integer,
       zip integer);
 
CREATE TABLE diagnoses (
    patient_id integer NOT NULL,
    site integer NOT NULL,
    year integer NOT NULL,
    month integer NOT NULL,
    visit_no integer NOT NULL,
    type_ integer NOT NULL,
    encounter_id integer NOT NULL,
    diag_src character varying NOT NULL,
    icd9 character varying NOT NULL,
    primary_ integer NOT NULL,
    timestamp_ timestamp without time zone,
    clean_icd9 character varying,
    major_icd9 character varying
);

DROP TABLE IF EXISTS cdiff_cohort;
CREATE TABLE cdiff_cohort AS (
       SELECT DISTINCT patient_id FROM diagnoses WHERE icd9 = '008.45');

DROP TABLE IF EXISTS mi_cohort;
CREATE TABLE mi_cohort AS (
       SELECT DISTINCT patient_id FROM diagnoses WHERE icd9 LIKE '410%');



 -- data used in test to simulate patient registry
DROP TABLE IF EXISTS cdiff_cohort_diagnoses;
CREATE TABLE cdiff_cohort_diagnoses AS SELECT * FROM diagnoses WHERE patient_id IN (SELECT * FROM cdiff_cohort); 
DROP TABLE IF EXISTS mi_cohort_diagnoses;
CREATE TABLE mi_cohort_diagnoses AS SELECT * FROM diagnoses  WHERE patient_id IN (SELECT * FROM mi_cohort); 

-- 

CREATE TABLE vitals (
	patient_id integer,
	height_timestamp timestamp,
	height_visit_no integer,
	height real,
	height_units character varying,
	weight_timestamp timestamp,
	weight_visit_no integer,
	weight real,
	weight_units character varying,
	bmi_timestamp timestamp,
	bmi_visit_no integer,
	bmi real,
	bmi_units character varying,
	pulse integer,
	systolic integer,
	diastolic integer ,
	bp_method character varying);       

CREATE TABLE medications (
    patient_id integer NOT NULL,
    site integer NOT NULL,
    year integer NOT NULL,
    month integer NOT NULL,
    medication character varying NOT NULL,
    dosage character varying NOT NULL,
    route character varying,
    timestamp_ timestamp without time zone
);

CREATE TABLE oblivious_partitioning_100 (
	a INT,
	b INT,
	c INT,
	d INT
);

CREATE TABLE oblivious_partitioning_1000 (
	a INT,
	b INT,
	c INT,
	d INT
);

CREATE TABLE oblivious_partitioning_10000 (
	a INT,
	b INT,
	c INT,
	d INT
);

CREATE TABLE oblivious_partitioning_100000 (
	a INT,
	b INT,
	c INT,
	d INT
);

CREATE TABLE oblivious_partitioning_1000000 (
	a INT,
	b INT,
	c INT,
	d INT
);

CREATE TABLE oblivious_partitioning_10000000 (
	a INT,
	b INT,
	c INT,
	d INT
);

CREATE TABLE left_deep_joins_1024 (
	a INT,
	b INT
);
