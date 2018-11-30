\echo 'medications'
select year, count(*) from medications group by year;
\echo 'diagnoses'
select year, count(*) from diagnoses group by year;
\echo 'vitals'
select year, count(*) from vitals group by year;
