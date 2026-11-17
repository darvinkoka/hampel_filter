#include "filters.h"


int SIZE = 9;
double NDEV = 2.0;
double K = 1.4826;


Node* createNode(double value) {
    Node *newNode = malloc(sizeof(Node));
    if (!newNode) {
        perror("Allocazione fallita");
        exit(EXIT_FAILURE);
    }
    newNode->value = value;
    newNode->nextFIFO = newNode->prevFIFO = NULL;
    newNode->nextSorted = newNode->prevSorted = NULL;
    newNode->nextDevSorted = newNode->prevDevSorted = NULL;
    return newNode;
}

//aggiorno i puntatori "Sorted" quando AGGIUNGO un elemento 
void insertSorted(Queue *q, Node *node) {
    if (!q->sortedHead) {
        q->sortedHead = node;
        return;
    }
    Node *current = q->sortedHead;
    Node *prev = NULL;
    while (current && current->value < node->value) {
        prev = current;
        current = current->nextSorted;
    }
    if (!prev) {
        node->nextSorted = q->sortedHead;
        q->sortedHead->prevSorted = node;
        q->sortedHead = node;
    } else {
        node->nextSorted = current;
        node->prevSorted = prev;
        prev->nextSorted = node;
        if (current)
            current->prevSorted = node;
    }
}

//aggiorno i puntatori "Sorted" quando ELIMINO un elemento 
void removeSorted(Queue *q, Node *node) {
    if (node->prevSorted)
        node->prevSorted->nextSorted = node->nextSorted;
    else
        q->sortedHead = node->nextSorted;
    
    if (node->nextSorted)
        node->nextSorted->prevSorted = node->prevSorted;
    
    node->nextSorted = node->prevSorted = NULL;
}

//aggiorno i puntatori "DevSorted" quando AGGIUNGO un elemento 
void insertDevSorted(Queue *q, Node *node) {
    if (!q->devSortedHead) {
        q->devSortedHead = node;
        return;
    }
    Node *current = q->devSortedHead;
    Node *prev = NULL;

    while (current && fabs(current->value - q->med->value) < fabs(node->value - q->med->value)) {
        prev = current;
        current = current->nextDevSorted;
    }
    if (!prev) {
        node->nextDevSorted = q->devSortedHead;
        q->devSortedHead->prevDevSorted = node;
        q->devSortedHead = node;
    } else {
        node->nextDevSorted = current;
        node->prevDevSorted = prev;
        prev->nextDevSorted = node;
        if (current)
            current->prevDevSorted = node;
    }
}

//aggiorno i puntatori "DevSorted" quando ELIMINO un elemento 
void removeDevSorted(Queue *q, Node *node) {
    if (node->prevDevSorted)
        node->prevDevSorted->nextDevSorted = node->nextDevSorted;
    else
        q->devSortedHead = node->nextDevSorted;
    
    if (node->nextDevSorted)
        node->nextDevSorted->prevDevSorted = node->prevDevSorted;
    
    node->nextDevSorted = node->prevDevSorted = NULL;
}

void initQueue(Queue *q) {
    q->fifoHead = NULL;
    q->fifoTail = NULL;
    q->sortedHead = NULL;
    q->devSortedHead = NULL;

    q->med = NULL;
    q->center = NULL;
    q->MAD = NULL;
    q->count = 0;
    q->variance = 0.;
    q->mean = 0.;
}

/*
void printQueue(Queue *q) {
    printf("FIFO order: ");
    Node *current = q->fifoHead;
    while (current) {
        printf("%.2f ", current->value);  
        current = current->nextFIFO;
    }
    printf("\n");
    
    printf("Sorted order: ");
    current = q->sortedHead;
    while (current) {
        printf("%.2f ", current->value);  
        current = current->nextSorted;
    }
    printf("\n");

    printf("Devsorted order: ");
    current = q->devSortedHead;
    while(current){
        printf("%.2f ", fabs(current->value - q->med->value));  
        current = current->nextDevSorted;
    }

    printf("\nMediana: %.2f\nCentro: %.2f\nMedia: %.2f Varianza: %.2f\nMAD: %.2f \n\n", 
          q->med->value,    
          q->center->value, 
          q->mean,          
          q->variance,
          fabs(q->MAD->value-q->med->value));       
}
*/
double findMedianFromHead(Queue *q) {
    if(q->count < SIZE) return -1; // Buffer non ancora pieno
    
    Node *current = q->sortedHead;
    for(int i = 0; i < (SIZE-1)/2; i++)
        current = current->nextSorted;
    
    return current->value;
}
 
double findMADFromHead(Queue *q) {
    if(q->count < SIZE) return -1; // Buffer non ancora pieno
    
    Node *current = q->devSortedHead;
    for(int i = 0; i < (SIZE-1)/2; i++){
        current = current->nextDevSorted;
    }
    return current->value;
}


//Aggiorna in modo incrementale (senza ricalcolare dall'inzio) media e varianza
void update_params(Queue *q, double value){
    double old_mean = q->mean;
    double n = q->count;
    if(q->count<SIZE){
        //aggiorno la media
        q->mean = (q->mean*n +  value)/(n+1);
        //aggiorno la deviazione standard
        if(q->count == 1){
            q->variance = (value - q->mean)*(value - q->mean) + (q->fifoTail->value - q->mean)*(q->fifoTail->value - q->mean);
        }
        else if(q->count > 1){
            q->variance = ((n - 1)*q->variance + n*old_mean*old_mean + value*value-(n+1)*q->mean*q->mean)/n;
        }
    } 
    else{
        q->mean = (q->mean*n - q->fifoTail->value + value)/n;
        q->variance = ((n - 1)*q->variance - q->fifoTail->value*q->fifoTail->value + n*old_mean*old_mean + value*value-n*q->mean*q->mean)/(n-1);
    }
        
}

//Aggiorno in modo incrementale la mediana, considera che l'ultimo elemento della della Fifo (fifotail) esce e il primo entra (fifoHead)
//performo l'update quando l'ultimo elemento della fifo non è stato ancora rimosso
void update_med(Queue *q){
        double value = q->fifoHead->value;
        Node *old = q->fifoTail;

        //AGGIORNO MEDIANA
        if(old==q->med){
            if(value <= q->med->prevSorted->value)
                q->med = q->med->prevSorted;
            else 
                q->med = q->med->nextSorted;

        } 
        else{
            if(old->value< q->med->value){
                if(value > q->med->value){ 
                        q->med = q->med->nextSorted;
                }
            }
            else if(old->value > q->med->value){
                if(value <= q->med->value){ 
                        q->med = q->med->prevSorted;
                }
            }
            else{
                // deve essere così per via della funzione hampel filter
                Node *current = old;
                while(current && current->value == q->med->value && current != q->med){
                    current = current->nextSorted;
                }
                if(!current || current != q->med){
                    if(value<=q->med->value)
                        q->med = q->med->prevSorted;
                }
                else{
                    if(value > q->med->value)
                        q->med = q->med->nextSorted;
                }
            }
            
        }

            q->center = q->center->prevFIFO;

        // Rimozione dalla lista ordinata
        removeSorted(q, old);

}

//Ricostruisce da capo i puntatori DevSorted perchè la mediana può cambiare ad ogni passo
void update_MAD(Queue *q) {
    // Reset dei puntatori DevSorted
    Node *current = q->fifoHead;
    for (int i = 0; i < q->count-1; i++) {
        current->prevDevSorted = NULL;
        current->nextDevSorted = NULL;
        current = current->nextFIFO;
    }
    q->devSortedHead = NULL;
    current = q->fifoHead;
    for (int i = 0; i < q->count-1; i++) {
        insertDevSorted(q, current);
        current = current->nextFIFO;
    }
    
    // Aggiorna il puntatore MAD scorrendo fino al nodo centrale
    current = q->devSortedHead;
    for (int i = 0; i < (q->count - 1) / 2; i++) {
        current = current->nextDevSorted;
    }
    q->MAD = current;
}

//Inserisce un nuovo elemento nella lista
void enqueue(Queue *q, double value) {

    Node *newNode = createNode(value);
    update_params(q, value);
    // Inserimento nella lista FIFO (in testa, per semplicità)
    newNode->nextFIFO = q->fifoHead;
    if (q->fifoHead)
        q->fifoHead->prevFIFO = newNode;
    q->fifoHead = newNode;
    if (!q->fifoTail)
        q->fifoTail = newNode;

    // Inserimento nella lista ordinata
    insertSorted(q, newNode);
    
    q->count++;

    //inizializzo la mediana e la MAD solo quando riempio il buffer (una volta sola!)
    if(q->count == SIZE){
        q->center = q->fifoHead;
        q->med = q->sortedHead;
        for(size_t i = 0; i<(SIZE-1)/2; i++){
            q->center = q->center->nextFIFO;
            q->med = q->med->nextSorted;
        }
        //inizializzo anche la lista con le deviazioni dalla mediana (non posso farlo prima perchè mi serve la mediana)
        Node *current = q->sortedHead;
              
        for(size_t i = 0; i<SIZE; i++){
            insertDevSorted(q, current);
            current = current->nextSorted;
        }
        
        q->MAD = q->devSortedHead;
        for(size_t i = 0; i<(SIZE-1)/2; i++){
            q->MAD = q->MAD->nextDevSorted;  
        }

    }

    // Se la coda supera SIZE, rimuovo l'elemento più vecchio (tail della FIFO)    
    if (q->count > SIZE) {

        Node *old = q->fifoTail;
        update_med(q);
        
        //dopo aver aggiornato la mediana elimino anche dalla FIFO l'ultimo elemento
        q->fifoTail = old->prevFIFO;
        q->fifoTail->nextFIFO = NULL;

        update_MAD(q);
        free(old);
        q->count--;
    }

    
}

double hampel_filter(Queue *q){

    double val = q->center->value;
    double mad = fabs(q->MAD->value - q->med->value);
    if(val > q->med->value + K*NDEV*mad || val < q->med->value - K*NDEV*mad){
        //aggiustare la lista ordinata
        removeSorted(q, q->center);
        q->center->value = q->med->value;
        insertSorted(q,q->center);
        if(val > q->med->value)
            q->med = q->med->prevSorted;
        //riaggiornare media e varianza
        double n = q->count;
        double old_mean = q->mean;

        q->mean = (q->mean*n - val + q->med->value)/n;
        q->variance = ((n - 1)*q->variance - val*val + n*old_mean*old_mean + (q->med->value)*(q->med->value)-n*q->mean*q->mean)/(n-1);
        update_MAD(q);
    }

    return q->center->value;
}


/*
int main(){
    
    Queue buffer;
    initQueue(&buffer);

    FILE *input = fopen("D:/2024/sailingteam/Ultrasonic Sensor/Ultrasonic Sensor/test nella vasca/simulatore_onde_1.csv", "r");
    if (!input) {
        perror("Errore apertura input.csv");
        return -1;
    }
    FILE *output = fopen("D:/2024/sailingteam/Ultrasonic Sensor/Ultrasonic Sensor/test nella vasca/output.csv", "w");
    if (!output) {
        perror("Errore apertura output.csv");
        fclose(input);
        return -1;
    }

    fprintf(output, "Reading,Median,Hampel,Mean,Variance,MAD/n");

    
    
    //char timestamp[32];
    double data_ret;
    int i=0;
    //while (fgets(line, sizeof(line), input)) 
    double val;
    while (fscanf(input, "%lf,", &val) == 1){        
        int conta_err = 0; 
        
        //GESTIONE DATI ERRATI

        if(i==0 && (isnan(val) || val<0)) {
            val = 0;
            conta_err++;
        }
        if(i>0 &&  (isnan(val) || val<0) && conta_err<15){
            val = buffer.fifoTail->value;
            conta_err++;
        }
        if(i>0 && (isnan(val) || val<0) && conta_err >= 15){
            printf("ERRORE: Troppe altezze minori di zero di fila");
            return -1;
        }
        conta_err = 0;

        enqueue(&buffer, val);

        if(buffer.count >= SIZE) {
            data_ret = hampel_filter(&buffer);
            fprintf(output, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n", 
                    val, 
                    buffer.med->value, 
                    data_ret, 
                    buffer.mean, 
                    buffer.variance, 
                    buffer.MAD->value);
            printQueue(&buffer);        
        }
        
        i++; 
    
     
    }

    fclose(input);
    fclose(output);
    return 0;
}

*/
