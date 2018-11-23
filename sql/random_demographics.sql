insert into demographics (patient_id, race, gender) select floor(random()*1000+1)::int, floor(random()*2+1), floor(random()*10+1) from generate_series(1,1000) s(i);
