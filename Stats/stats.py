import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Configuración de estilo
sns.set(style="whitegrid")

# Leer archivo
file_path = "../latency_log.txt"
df = pd.read_csv(file_path, delim_whitespace=True, header=None,
                 names=["pe_src", "QoS", "operation", "size", "bytes", "latency"])

# --------------------- GRÁFICO 1: Latencia por PE ---------------------
plt.figure(figsize=(14, 7))
sns.barplot(data=df, x="pe_src", y="latency", hue="operation", ci=None)
plt.title("Latencia por PE")
plt.xlabel("Processing Element (PE)")
plt.ylabel("Latencia (ciclos)")
plt.grid(True, axis='y', linestyle="--", alpha=0.5)
plt.legend(title="Operación", bbox_to_anchor=(1.05, 1), loc='upper left')
plt.tight_layout()
plt.show()

# --------------------- GRÁFICO 2: IPC por PE ---------------------
instructions_per_pe = df.groupby("pe_src")["operation"].count()
cycles_per_pe = df.groupby("pe_src")["latency"].sum()
ipc_per_pe = instructions_per_pe / cycles_per_pe

ipc_df = pd.DataFrame({
    "pe_src": ipc_per_pe.index,
    "IPC": ipc_per_pe.values
})

plt.figure(figsize=(12, 6))
sns.barplot(data=ipc_df, x="pe_src", y="IPC", palette="viridis")
plt.title("Instrucciones por Ciclo (IPC) por PE")
plt.xlabel("Processing Element (PE)")
plt.ylabel("IPC")
plt.grid(True, axis='y', linestyle='--', alpha=0.5)
plt.tight_layout()
plt.show()

# --------------------- GRÁFICO 3: Bandwidth por PE ---------------------
df_bandwidth = df[(df["latency"] > 0) & ((df["size"] != 0) | (df["bytes"] != 0))].copy()
df_bandwidth["valid_value"] = df_bandwidth.apply(
    lambda row: row["size"] if row["size"] != 0 else row["bytes"],
    axis=1
)
df_bandwidth["bandwidth"] = (df_bandwidth["valid_value"] * 4) / df_bandwidth["latency"]

plt.figure(figsize=(12, 6))
sns.barplot(data=df_bandwidth, x="pe_src", y="bandwidth", hue="operation", ci=None)
plt.title("Ancho de Banda por PE (Bytes por Ciclo)")
plt.xlabel("Processing Element (PE)")
plt.ylabel("Bandwidth (Bytes/ciclo)")
plt.grid(True, axis='y', linestyle="--", alpha=0.5)
plt.legend(title="Operación", bbox_to_anchor=(1.05, 1), loc='upper left')
plt.tight_layout()
plt.show()

# --------------------- GRÁFICO 4: Latencia Promedio por Operación ---------------------
latencia_promedio = df.groupby("operation")["latency"].mean().reset_index()

plt.figure(figsize=(10, 6))
sns.barplot(data=latencia_promedio, y="operation", x="latency", palette="Set2")
plt.title("Latencia Promedio por Tipo de Instrucción")
plt.xlabel("Latencia Promedio (ciclos)")
plt.ylabel("Operación")
plt.grid(True, axis='x', linestyle='--', alpha=0.5)
plt.tight_layout()
plt.show()

# --------------------- GRÁFICO 5: Latencia Promedio por QoS ---------------------
# Convertir QoS de hexadecimal a decimal si es necesario
df["QoS_dec"] = df["QoS"].apply(lambda x: int(str(x), 16) if isinstance(x, str) and str(x).startswith("0x") else int(x))

# Calcular latencia promedio por QoS
qos_latency = df.groupby("QoS_dec")["latency"].mean().reset_index()

plt.figure(figsize=(12, 6))
sns.barplot(data=qos_latency, x="QoS_dec", y="latency", palette="coolwarm")
plt.title("Latencia Promedio por Nivel de QoS")
plt.xlabel("QoS (decimal)")
plt.ylabel("Latencia Promedio (ciclos)")
plt.grid(True, axis='y', linestyle='--', alpha=0.5)
plt.tight_layout()
plt.show()
