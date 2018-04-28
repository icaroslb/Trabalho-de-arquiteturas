import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
 
public class Main {
 
  public static void main(String[] args) {
    
	 String formula= "(	((\\S+)\\s([A-Z]+)\\s([a-zA-Z]+))|	(([A-Z]+)\\s(\\S+)))"; 
    String nome = "Teste.txt";
    try {
      FileReader arq = new FileReader(nome);
      BufferedReader lerArq = new BufferedReader(arq);
 
      String linha = lerArq.readLine(); 
      while (linha != null) {
    	  if(linha.matches(formula)) {
        System.out.println(linha);
    	  }
        linha = lerArq.readLine(); // lê da segunda até a última linha
      }
 
      arq.close();
    } catch (IOException e) {
        System.err.printf("Erro na abertura do arquivo: %s.\n",
          e.getMessage());
    }
 
    System.out.println();
  }
}