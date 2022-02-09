model test2
  //Model without algebraic loops containing connections between different indices in one direction
  constant Integer N=1000;
  Real x1[N], a1[N];
  Real x2[N], a2[N];
  Real x3[N], a3[N];
  Real x4[N], a4[N];
  Real x5[N], a5[N];
  Real x6[N], a6[N];
  Real x7[N], a7[N];
/*
  Real x8[N], a8[N];
  Real x9[N], a9[N];
  Real x10[N], a10[N];
*/
equation
  a1[1]=x1[1];
  der(x1[1])=1-a1[1];
  for i in 2:N loop
    a1[i]=a1[i-1]-x1[i];
    der(x1[i])=a1[i];
  end for;  

  a2[1]=x2[1];
  der(x2[1])=1-a2[1];
  for i in 2:N loop
    a2[i]=a2[i-1]-x2[i];
    der(x2[i])=a2[i];
  end for;

  a3[1] = x3[1];
  der(x3[1]) = 1 - a3[1];
  for i in 2:N loop
    a3[i] = a3[i - 1] - x3[i];
    der(x3[i]) = a3[i];
  end for;

  a4[1] = x4[1];
  der(x4[1]) = 1 - a4[1];
  for i in 2:N loop
    a4[i] = a4[i - 1] - x4[i];
    der(x4[i]) = a4[i];
  end for;

  a5[1] = x5[1];
  der(x5[1]) = 1 - a5[1];
  for i in 2:N loop
    a5[i] = a5[i - 1] - x5[i];
    der(x5[i]) = a5[i];
  end for;

  a6[1] = x6[1];
  der(x6[1]) = 1 - a6[1];
  for i in 2:N loop
    a6[i] = a6[i - 1] - x6[i];
    der(x6[i]) = a6[i];
  end for;

  a7[1] = x7[1];
  der(x7[1]) = 1 - a7[1];
  for i in 2:N loop
    a7[i] = a7[i - 1] - x7[i];
    der(x7[i]) = a7[i];
  end for;

/*
  a8[1] = x8[1];
  der(x8[1]) = 1 - a8[1];
  for i in 2:N loop
    a8[i] = a8[i - 1] - x8[i];
    der(x8[i]) = a8[i];
  end for;

  a9[1] = x9[1];
  der(x9[1]) = 1 - a9[1];
  for i in 2:N loop
    a9[i] = a9[i - 1] - x9[i];
    der(x9[i]) = a9[i];
  end for;

  a10[1] = x10[1];
  der(x10[1]) = 1 - a10[1];
  for i in 2:N loop
    a10[i] = a10[i - 1] - x10[i];
    der(x10[i]) = a10[i];
  end for;
*/
end test2;
