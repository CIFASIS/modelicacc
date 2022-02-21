model matching1
  //Model without algebraic loops and without connections between different indices
  constant Integer N=100;
  Real a[N], x[N];
equation
  for i in 1:N loop
    a[i] = x[i];
    der(x[i]) = 1 - a[i];
  end for;  
end matching1;
