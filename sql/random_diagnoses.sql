insert into diagnoses (patient_id, diag_src, site, year, month, visit_no, type_, encounter_id, icd9, primary_, clean_icd9, major_icd9, timestamp_) select floor(random()*1000+1)::int, 'hd', i,i,i,i,i,i,i,i,i,i,to_timestamp(i)::date from generate_series(1,1000) s(i);

insert into diagnoses (patient_id, diag_src, site, year, month, visit_no, type_, encounter_id, icd9, primary_, clean_icd9, major_icd9, timestamp_) select 0, 'hd', i,i,i,i,i,i,i,i,i,i,to_timestamp(i)::date from generate_series(1,1000) s(i);

insert into diagnoses (patient_id, diag_src, site, year, month, visit_no, type_, encounter_id, icd9, primary_, clean_icd9, major_icd9, timestamp_) select floor(random()*1000+1)::int, 'hd', 4,2006,i,i,i,i,'008.45',i,i,floor(random()*10+1)::int,to_timestamp(i)::date from generate_series(1,100) s(i);
