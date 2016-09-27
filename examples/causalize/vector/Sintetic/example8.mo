model A
	Real b[10];
equation
	//sum(b)=123;
	b[1]+	b[2]+	b[3]+	b[4]+	b[5]+	b[6]+	b[7]+	b[8]+	b[9]+	b[10]=53;
	for i in 1:3 loop
		b[i]=123;
	end for;
	for i in 5:10 loop
		b[i]=123;
	end for;

end A;

// Sum is not working


