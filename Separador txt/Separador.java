
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.*;
 
public class Separador {
	Map<Integer,String> filtro=new HashMap<>();
	
	Separador(){
		
	}

	boolean Filtrador(String onde) {
		Integer cont=0 ;
		String formula= "(	((\\S+)\\s([A-Z]+)\\s([a-zA-Z]+))|	(([A-Z]+)\\s(\\S+)))"; 
		String nome = onde;
		try {
			FileReader arq = new FileReader(nome);
			BufferedReader lerArq = new BufferedReader(arq);
 
			String linha = lerArq.readLine(); 
			while (linha != null) {
				linha = lerArq.readLine();
				if(linha.matches(formula)) {
					filtro.put(cont,linha.replaceAll("\t", ""));
					System.out.println(linha);
					cont++;
				}else {
					System.out.println("FIM");
					return false;
				}
			}
			lerArq.close();
			arq.close();
		} catch (IOException e) {
			System.err.printf("Erro na abertura do arquivo: %s.\n",
					e.getMessage());
		}
		
		System.out.println();
		return true;
	}
}
