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
    major_icd9 integer
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
	b INT
);

CREATE TABLE oblivious_partitioning_1000 (
	a INT,
	b INT
);

CREATE TABLE oblivious_partitioning_10000 (
	a INT,
	b INT
);

CREATE TABLE oblivious_partitioning_100000 (
	a INT,
	b INT
);

CREATE TABLE oblivious_partitioning_1000000 (
	a INT,
	b INT
);

CREATE TABLE oblivious_partitioning_10000000 (
	a INT,
	b INT
);

CREATE TABLE left_deep_joins_512 (
	a INT,
	b INT
);

CREATE TABLE left_deep_joins_1024 (
	a INT,
	b INT
);
CREATE TABLE gen_test_a_h1_c (
	a VARCHAR,
	b VARCHAR
);

CREATE TABLE gen_test_a_h2_c (
	a VARCHAR,
	b VARCHAR
);
CREATE TABLE gen_test_a_h3_c (
	a VARCHAR,
	b VARCHAR
);
CREATE TABLE gen_test_a_h4_c (
	a VARCHAR,
	b VARCHAR
);

CREATE TABLE gen_test_a_h1 (
	a INT,
	b INT
);

CREATE TABLE gen_test_a_h2 (
	a INT,
	b INT
);
CREATE TABLE gen_test_a_h3 (
	a INT,
	b INT
);
CREATE TABLE gen_test_a_h4 (
	a INT,
	b INT
);
CREATE TABLE gen_test_b_h1 (
	a INT,
	b INT
);

CREATE TABLE gen_test_b_h2 (
	a INT,
	b INT
);
CREATE TABLE gen_test_b_h3 (
	a INT,
	b INT
);
CREATE TABLE gen_test_b_h4 (
	a INT,
	b INT
);

CREATE TABLE gen_test_c_h1 (
	a INT,
	b INT
);

CREATE TABLE gen_test_c_h2 (
	a INT,
	b INT
);
CREATE TABLE gen_test_c_h3 (
	a INT,
	b INT
);
CREATE TABLE gen_test_c_h4 (
	a INT,
	b INT
);
CREATE TABLE gen_test_d_h1 (
	a INT,
	b INT
);
CREATE TABLE gen_test_d_h2 (
	a INT,
	b INT
);
CREATE TABLE gen_test_d_h3 (
	a INT,
	b INT
);
CREATE TABLE gen_test_d_h4 (
	a INT,
	b INT
);

CREATE TABLE left_deep_joins_4096 (
	a INT,
	b INT
);

CREATE TABLE left_deep_joins_25200 (
	a INT,
	b INT
);
