insert into vitals (patient_id, pulse) select floor(random()*1000+1)::int, floor(random()*100+1)::int from generate_series(1,1000) s(i);
