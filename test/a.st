fn add(a:Event<i32> , b:Event<i32>){
  return a+b;
}
fn main(a:i64,b:i64){
  c = a + b;
  d = c * 2;
  e = d + 1;
  x = add(d,e);
  stx = latestEventToState(x);
  y = stateChangeEvents(stx);
  return stx;
}
