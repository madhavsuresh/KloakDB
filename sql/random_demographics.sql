insert into demographics (patient_id) select floor(random()*1000+1)::int from generate_series(1,1000) s(i);
