log("Starting ...\n");

accum = 0;
count = 0;
while (count < 1545)
//while (count < 2)
{
	left_edge   = -420;
	right_edge  =  300;
	top_edge    =  300;
	bottom_edge = -300;
	x_step      =  7;
	y_step      =  15;

	max_iter    =  200;

	y0 = top_edge;
	while (y0 > bottom_edge)
	{
		x0 = left_edge;
		while (x0 < right_edge)
		{
			y = 0;
			x = 0;
			the_char = 32;
			x_x = 0;
			y_y = 0;
			i = 0;
			while (i < max_iter && x_x + y_y <= 800)
			{
				x_x = (x * x) / 200;
				y_y = (y * y) / 200;
				if (x_x + y_y > 800 )
				{
					the_char = 48 + i;
					if (i > 9)
					{
						the_char = 64;
					}
				}
				else
				{
					temp = x_x - y_y + x0;
					if ((x < 0 && y > 0) || (x > 0 && y < 0))
					{
						y = (-1 * ((-1 * (x * y)) / 100)) + y0;
					}
					else
					{
						y = x * y / 100 + y0;
					}
					
					x = temp;
				}

				i = i + 1;
			}
			
			accum = accum + the_char;

			x0 = x0 + x_step;
		}
		
		y0 = y0 - y_step;
	}
	
	if (count % 300 == 0)
	{
		log(accum);
	}

	count = count + 1;
}

log(accum);
