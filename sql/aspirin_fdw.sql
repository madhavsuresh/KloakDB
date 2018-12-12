WITH medications_all AS (
	SELECT patient_id FROM vdb1.meds_ex where medication like '%ASPIRIN%' and
					year = {0}
	UNION ALL
	SELECT patient_id FROM vdb2.meds_ex where medication like '%ASPIRIN%' and
					year = {0}
	UNION ALL
	SELECT patient_id FROM vdb3.meds_ex where medication like '%ASPIRIN%' and
					year = {0}
	UNION ALL
	SELECT patient_id FROM vdb4.meds_ex where medication like '%ASPIRIN%' and
					year = {0}
),

diagnoses_all AS (
	SELECT patient_id FROM vdb1.diagnoses where year = {0} -- and where diag_src = 'hd'
	UNION ALL
	SELECT patient_id FROM vdb2.diagnoses where year = {0}
	UNION ALL
	SELECT patient_id FROM vdb3.diagnoses where year = {0}
	UNION ALL
	SELECT patient_id FROM vdb4.diagnoses where year = {0}
),

demo_all AS (
	SELECT patient_id, race, gender FROM vdb1.dem_ex 
	UNION ALL
	SELECT patient_id, race, gender FROM vdb2.dem_ex
	UNION ALL
	SELECT patient_id, race, gender FROM vdb3.dem_ex
	UNION ALL 
	SELECT patient_id, race, gender FROM vdb4.dem_ex ),

vitals_all AS (
	SELECT pulse, patient_id from vdb1.vit_ex where year = {0}
	UNION ALL
	SELECT pulse, patient_id from vdb2.vit_ex where year = {0}
	UNION ALL
	SELECT pulse, patient_id from vdb3.vit_ex where year = {0}
	UNION ALL
	SELECT pulse, patient_id from vdb4.vit_ex where year = {0}
	)

select gender, race, avg(pulse) FROM medications_all m, diagnoses_all di, demo_all de, vitals_all vi where
di.patient_id = de.patient_id and
di.patient_id = vi.patient_id and 
di.patient_id = m.patient_id

group by gender, race;


