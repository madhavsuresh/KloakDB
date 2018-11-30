WITH medications_all AS (
	SELECT patient_id FROM vdb1_medications where medication like '%ASPIRIN%' and
					year = {0}
	UNION ALL
	SELECT patient_id FROM vdb2_medications where medication like '%ASPIRIN%' and
					year = {0}
	UNION ALL
	SELECT patient_id FROM vdb3_medications where medication like '%ASPIRIN%' and
					year = {0}
	UNION ALL
	SELECT patient_id FROM medications where medication like '%ASPIRIN%' and
					year = {0}
),

diagnoses_all AS (
	SELECT patient_id FROM vdb1_diagnoses where year = {0} -- and where diag_src = 'hd'
	UNION ALL
	SELECT patient_id FROM vdb2_diagnoses where year = {0}
	UNION ALL
	SELECT patient_id FROM vdb3_diagnoses where year = {0}
	UNION ALL
	SELECT patient_id FROM diagnoses where year = {0}
),

demo_all AS (
	SELECT patient_id, race, gender FROM vdb1_demographics 
	UNION ALL
	SELECT patient_id, race, gender FROM vdb2_demographics
	UNION ALL
	SELECT patient_id, race, gender FROM vdb3_demographics
	UNION ALL 
	SELECT patient_id, race, gender FROM demographics ),

vitals_all AS (
	SELECT pulse, patient_id from vdb1_vitals where year = {0}
	UNION ALL
	SELECT pulse, patient_id from vdb2_vitals where year = {0}
	UNION ALL
	SELECT pulse, patient_id from vdb3_vitals where year = {0}
	UNION ALL
	SELECT pulse, patient_id from vitals where year = {0}
	)

select gender, race, avg(pulse) FROM medications_all m, diagnoses_all di, demo_all de, vitals_all vi where
di.patient_id = de.patient_id and
di.patient_id = vi.patient_id and 
di.patient_id = m.patient_id

group by gender, race;


