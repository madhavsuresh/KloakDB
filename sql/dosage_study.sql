select d.patient_id from diagnoses d, medications m where d.patient_id=m.patient_id and medication='aspirin' and icd9 = 'internal bleeding' and dosage='325mg';

