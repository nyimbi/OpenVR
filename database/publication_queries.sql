Title: Occupations By Gender


Query:
SELECT occupation,SUM(IF(`gender`='Male',1,0)) AS 'Male',SUM(IF(`gender`='Female',1,0)) AS 'Female' from registrations GROUP by occupation;
----------------------------------------------------------------------------------------------------  
 

Title: Count By LGA
Query:
SELECT s.name as state, l.name as LGA,COUNT(*) as Total_Voters FROM registrations reg
LEFT JOIN polling_units p ON p.id=reg.polling_unit_id LEFT JOIN registration_areas r ON r.id=p.registration_area_id
	LEFT JOIN local_governments l ON l.id=r.local_government_id LEFT JOIN states s ON s.id=l.state_id 
	GROUP BY l.id;

----------------------------------------------------------------------------------------------------  

Title: Count by Name Occurence:
Query:
SELECT	last_name,COUNT(*) as Total_Voters FROM registrations GROUP by last_name ORDER BY COUNT(*) DESC LIMIT 20;

----------------------------------------------------------------------------------------------------  

Title: Count By Year of Birth
Query:
SELECT	dob_year, (2012-dob_year) as CurrentAge,COUNT(*) as Total_Voters FROM registrations GROUP by dob_year ORDER BY COUNT(*) DESC LIMIT 20;

----------------------------------------------------------------------------------------------------  


Title: Top 10 Polling Unit by Voter Strength

Query:
SELECT s.name as state, l.name as LGA,r.name as WARD,p.name as PU,reg.delimitation,COUNT(*) as Total_Voters FROM registrations reg
LEFT JOIN polling_units p ON p.id=reg.polling_unit_id LEFT JOIN registration_areas r ON r.id=p.registration_area_id
	LEFT JOIN local_governments l ON l.id=r.local_government_id LEFT JOIN states s ON s.id=l.state_id 
	GROUP BY p.id ORDER BY COUNT(*) DESC LIMIT 10;


----------------------------------------------------------------------------------------------------  

Title: Top 10 Ward by Voter Strength

Query:
SELECT s.name as state, l.name as LGA,r.name as WARD,COUNT(*) as Total_Voters FROM registrations reg
LEFT JOIN polling_units p ON p.id=reg.polling_unit_id LEFT JOIN registration_areas r ON r.id=p.registration_area_id
	LEFT JOIN local_governments l ON l.id=r.local_government_id LEFT JOIN states s ON s.id=l.state_id 
	GROUP BY r.id ORDER BY COUNT(*) DESC LIMIT 10;

----------------------------------------------------------------------------------------------------
Title: Gender BY LGA

Query:
SELECT s.name as state, l.name as LGA,SUM(IF(`gender`='Male',1,0)) AS 'Male',SUM(IF(`gender`='Female',1,0)) AS 'Female' FROM registrations reg
LEFT JOIN polling_units p ON p.id=reg.polling_unit_id LEFT JOIN registration_areas r ON r.id=p.registration_area_id
	LEFT JOIN local_governments l ON l.id=r.local_government_id LEFT JOIN states s ON s.id=l.state_id 
	GROUP BY l.id;



----------------------------------------------------------------------------------------------------
Title: Age Group BY LGA

Query:
SELECT s.name as state, l.name as LGA,
SUM(IF(`dob_year` BETWEEN 1987 AND 1994,1,0)) AS '18-25',
SUM(IF(`dob_year` BETWEEN 1981 AND 1986,1,0)) AS '26-31',
SUM(IF(`dob_year` BETWEEN 1972 AND 1980,1,0)) AS '32-40',
SUM(IF(`dob_year` BETWEEN 1966 AND 1971,1,0)) AS '41-46',
SUM(IF(`dob_year` BETWEEN 1957 AND 1965,1,0)) AS '47-55',
SUM(IF(`dob_year` < 1957,1,0)) AS '56 and Above'


FROM registrations reg
LEFT JOIN polling_units p ON p.id=reg.polling_unit_id LEFT JOIN registration_areas r ON r.id=p.registration_area_id
	LEFT JOIN local_governments l ON l.id=r.local_government_id LEFT JOIN states s ON s.id=l.state_id 
	GROUP BY l.id;


----------------------------------------------------------------------------------------------------
Title: Age Group Distribution

Query:
SELECT s.name as state,
SUM(IF(`dob_year` BETWEEN 1987 AND 1994,1,0)) AS '18-25',
SUM(IF(`dob_year` BETWEEN 1981 AND 1986,1,0)) AS '26-31',
SUM(IF(`dob_year` BETWEEN 1972 AND 1980,1,0)) AS '32-40',
SUM(IF(`dob_year` BETWEEN 1957 AND 1971,1,0)) AS '41-55',
SUM(IF(`dob_year` < 1957,1,0)) AS '56 and Above'


FROM registrations reg
LEFT JOIN polling_units p ON p.id=reg.polling_unit_id LEFT JOIN registration_areas r ON r.id=p.registration_area_id
	LEFT JOIN local_governments l ON l.id=r.local_government_id LEFT JOIN states s ON s.id=l.state_id ;

