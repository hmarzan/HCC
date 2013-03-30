import "stdhpp\stdapi.hcc";


class Program
{
	public static void main()
	{
	using print = Console::WriteLn;
		double temp = 0.0;
		for(int i=0; i < 50; i++)
		{
			temp += 0.1;
			print(temp);
		}
	}
};