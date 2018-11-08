select gender, race, avg(pulse) from  demographics de, diagnoses di, vitals v, medications m where m.medication = 'aspirin' and di.diag_src = 'hd' and de.patient_id = di.patient_id and di.patient_id = v.patient_id and m.patient_id = di.patient_id group by gender, race;

