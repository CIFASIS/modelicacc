model Modelica
	model A
	    connector Pin
		Real i;
		flow Real v;
	    end Pin;
	    Pin a,b,c,d,f,g;
	equation
		connect(a,b);
		connect(a,c);
		connect(d,f);
		connect(f,g);
	end A;
end Modelica;
