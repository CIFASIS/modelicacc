model A
	connector Pin
		Real a;
		flow Real v;
	end Pin;
	model B
		Pin a;
		Pin b;
	equation
		connect (a,b);
	end B;
	Pin c;		
	B t;
equation
	connect(t.a,c);
end A;
