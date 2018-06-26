import matplotlib.pyplot as plt

file = open("output.txt","r");
coordenadas = file.read().split("\n");
x = []
y = []
for coordenada in coordenadas:
    coordenada = coordenada.split(" ");
    if(len(coordenada) > 1):
        x.append(float(coordenada[0]));
        y.append(float(coordenada[1]));
plt.title("grafico");
print(x);
plt.plot(x,y,".");
plt.axis((-5,0,-5,0));
plt.show();
