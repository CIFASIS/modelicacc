model matching8
   //Model with small algebraic loops and with connections between different indices in two directions
   constant Integer N=100;
  Real a[N], b[N], x[N];
equation
  a[1] = 2 * x[1] - b[1];
  a[1] = 2 * b[1] - x[1];
  der(x[1]) = 1 - 2 * a[1] + a[2];
  a[N] = 2 * x[N] - b[N];
  a[N] = 2 * b[N] - x[N];
  der(x[N]) = 1 - 2 * a[N] + a[N - 1];
  for i in 2:N-1 loop
    a[i] = 2 * x[i] - b[i];
    a[i] = 2 * b[i] - x[i];
    der(x[i]) = a[i - 1] - 2 * a[i] + a[i + 1];
  end for;  
end matching8;
