model matching3
  //Model with small algebraic loops and without connections between different indices
  constant Integer N = 100;
  Real a[N], b[N], x[N];
equation
  for i in 1:N loop
    a[i] = 2 * x[i] - b[i];
    a[i] = 2 * b[i] - x[i];
    der(x[i]) = 1 - a[i];
  end for;  
end matching3;
