local function expLoop(n)

    local sum = 0
    for i = 0, n do
      sum = sum + math.exp(1./(1.+i))
    end
    return sum
end

for i = 0, 19 do
	expLoop(10000000)
end
