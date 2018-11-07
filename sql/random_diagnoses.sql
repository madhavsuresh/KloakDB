insert into diagnoses (patient_id, diag_src, site, year, month, visit_no, type_, encounter_id, icd9, primary_, clean_icd9, major_icd9, timestamp_) select floor(random()*1000+1)::int, 'hd', i,i,i,i,i,i,i,i,i,i,to_timestamp(i)::date from generate_series(1,1000) s(i);

