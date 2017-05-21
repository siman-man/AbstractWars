locality = 3.0
dist = 100
sp = 0

nums = [10, 100, 1000]

nums.each do |n|
  sp = (1.0 / n) ** locality
  puts sp
end
