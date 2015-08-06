model airconds
	constant Integer N = 10;
	parameter Real CAP[N], RES[N], POT[N], THA = 32,pmax=0;
	Real th[N];
	discrete Real ptotal;
	discrete Real on[N];
	discrete Real tref[N];
	discrete Real nextSample[N];
	discrete Real noise[N];

	
	initial algorithm
	for i in 1:N loop
		th[i] := 4.0;
		CAP[i] := 100.0;
		RES[i] := 0.4;
		POT[i] := 2.0;
		pmax:=pmax+POT[i];
		nextSample[i] := 1;
		noise[i] := 2.0;
		tref[i] := 20;
	end for;


	equation
	for i in 1:N loop
		der(th[i]) = (THA/RES[i]-POT[i]*on[i]-th[i]/RES[i]+noise[i]/RES[i])/CAP[i];
	end for;
	
	algorithm
	for i in 1:N loop
		when th[i] - tref[i] + on[i] - 0.5 > 0 then
				on[i] := 1;
				ptotal := ptotal + POT[i];
		elsewhen th[i] - tref[i] + on[i] - 0.5 < 0 then
			if time > 0 then
				on[i] := 0;
				ptotal := ptotal - POT[i];
			end if;
		end when;
	end for;
	for i in 1:N loop
		when time > 1000 then
			tref[i] := 20.5;
		end when;
		when time > 2000 then
			tref[i] := 20;
		end when;
	end for;
	for i  in 1:N loop
		when time > nextSample[i] then
			nextSample[i] := nextSample[i]+1;
			noise[i] := 2.0;
		end when;
	end for;
end airconds;
