/* Model for causalization testing =============================================
 * Description: model that matches an equation array "irregularly". That is,
   different variables or occurences of the same variable are matched in the
   same array equation. This test was devised to to debug HorizontalSorting.
==============================================================================*/

model split_matching
  constant Integer N = 100;
  Real a[N];
equation
  a[50] = 0;
  for i in 1:N-1 loop
    a[i] + a[i+1] = 0;
  end for;
end split_matching;
