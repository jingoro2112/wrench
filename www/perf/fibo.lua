function fibonacci( n ) 
	if (n <= 1) then return n end
	return fibonacci(n - 2) + fibonacci(n - 1);
end

for i=0,38,1 do
--	print( fibonacci(i) );
	fibonacci(i);
end
