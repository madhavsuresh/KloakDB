insert into medications (medication, patient_id, year, month, dosage, route, timestamp_, site) select 'aspirin', floor(random() * 1000+1)::int, i,i,i,i,to_timestamp(i)::date, i  from generate_series(1,1000) s(i);

