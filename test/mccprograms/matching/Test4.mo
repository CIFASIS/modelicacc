model Test4
  constant Integer N = 100000;
  Real a1[N], x1[N], b1[N];
  Real a2[N], x2[N], b2[N];
/*
  Real a3[N], x3[N], b3[N];
  Real a4[N], x4[N], b4[N];
  Real a5[N], x5[N], b5[N];
  Real a6[N], x6[N], b6[N];
  Real a7[N], x7[N], b7[N];
  Real a8[N], x8[N], b8[N];
*/
equation 
  for i in 2:N-1 loop
    der(x1[i]) = a1[i] - x1[i];
    a1[i + 1] = a1[i] + b1[i];
    b1[i] = x1[i - 1];
  end for;
  b1[1] = 0;
  b1[N] = x1[N - 1];
  der(x1[1]) = a1[1] - x1[1];
  der(x1[N]) = a1[N] - x1[N];
  a1[1] = 1;
  a1[N] = a1[N - 1] + b1[N - 1];

  for i in 2:N-1 loop
    der(x2[i]) = a2[i] - x2[i];
    a2[i + 1] = a2[i] + b2[i];
    b2[i] = x2[i - 1];
  end for;
  b2[1] = 0;
  b2[N] = x2[N - 1];
  der(x2[1]) = a2[1] - x2[1];
  der(x2[N]) = a2[N] - x2[N];
  a2[1] = 1;
  a2[N] = a2[N - 1] + b2[N - 1];
/*
  for i in 2:N-1 loop
    der(x3[i]) = a3[i] - x3[i];
    a3[i + 1] = a3[i] + b3[i];
    b3[i] = x3[i - 1];
  end for;
  b3[1] = 0;
  b3[N] = x3[N - 1];
  der(x3[1]) = a3[1] - x3[1];
  der(x3[N]) = a3[N] - x3[N];
  a3[1] = 1;
  a3[N] = a3[N - 1] + b3[N - 1];

  for i in 2:N-1 loop
    der(x4[i]) = a4[i] - x4[i];
    a4[i + 1] = a4[i] + b4[i];
    b4[i] = x4[i - 1];
  end for;
  b4[1] = 0;
  b4[N] = x4[N - 1];
  der(x4[1]) = a4[1] - x4[1];
  der(x4[N]) = a4[N] - x4[N];
  a4[1] = 1;
  a4[N] = a4[N - 1] + b4[N - 1];

  for i in 2:N-1 loop
    der(x5[i]) = a5[i] - x5[i];
    a5[i + 1] = a5[i] + b5[i];
    b5[i] = x5[i - 1];
  end for;
  b5[1] = 0;
  b5[N] = x5[N - 1];
  der(x5[1]) = a5[1] - x5[1];
  der(x5[N]) = a5[N] - x5[N];
  a5[1] = 1;
  a5[N] = a5[N - 1] + b5[N - 1];

  for i in 2:N-1 loop
    der(x6[i]) = a6[i] - x6[i];
    a6[i + 1] = a6[i] + b6[i];
    b6[i] = x6[i - 1];
  end for;
  b6[1] = 0;
  b6[N] = x6[N - 1];
  der(x6[1]) = a6[1] - x6[1];
  der(x6[N]) = a6[N] - x6[N];
  a6[1] = 1;
  a6[N] = a6[N - 1] + b6[N - 1];

  for i in 2:N-1 loop
    der(x7[i]) = a7[i] - x7[i];
    a7[i + 1] = a7[i] + b7[i];
    b7[i] = x7[i - 1];
  end for;
  b7[1] = 0;
  b7[N] = x7[N - 1];
  der(x7[1]) = a7[1] - x7[1];
  der(x7[N]) = a7[N] - x7[N];
  a7[1] = 1;
  a7[N] = a7[N - 1] + b7[N - 1];

  for i in 2:N-1 loop
    der(x8[i]) = a8[i] - x8[i];
    a8[i + 1] = a8[i] + b8[i];
    b8[i] = x8[i - 1];
  end for;
  b8[1] = 0;
  b8[N] = x8[N - 1];
  der(x8[1]) = a8[1] - x8[1];
  der(x8[N]) = a8[N] - x8[N];
  a8[1] = 1;
  a8[N] = a8[N - 1] + b8[N - 1];
*/
end Test4;
