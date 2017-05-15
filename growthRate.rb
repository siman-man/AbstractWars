def attack
  base = 10
  power = 1.2
  opower = 1.0
  tp = 1.0

  base = [0, base - (tp / power).ceil.to_i].max
end

puts attack
