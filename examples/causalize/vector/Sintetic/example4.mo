model A
	Real b[10];
equation
	b[1]=345;
	for i in 1:9 loop
		b[i] + b[i+1] = 123;
	end for;
end A;

// Modify cancausalize
