#Elastic Optical Network 程式說明

## Introduction

程式主要分成下列11個檔案:
- auxiliary.hpp
- auxiliary.cpp
- graph.hpp
- graph.cpp
- light_path.hpp
- light_path.cpp
- spectrum.hpp
- spectrum.cpp
- traffic.hpp
- traffic.cpp
- simulator.cpp

其中 .hpp 檔是定義了個別 class 的 header 檔
.cpp 檔則是對應的 function definition

simulator.cpp 則是整個程式的中樞，利用其他檔案提供的物件來進行模擬
下面會對每個檔案分別作說明

## auxiliary.hpp

宣告所有 auxiliary graph 需要的物件(ex. auxiliary node, auxiliary link)

### Auxiliary_info

一個 struct 用來存放所有要帶進輔助圖的變數(ex. number of physical nodes, number of slots per physical link)

### Aux_node

auxiliary node 的 class, 使用了兩個 doubly linked list 來記錄所有連入及連出的 auxiliary link

- data
  - `Aux_node_type`: enumeration of types of auxiliary node
  - `type`: the type of auxiliary node (ex. adding node dropping node)
  - `phy_id`: 這個 auxiliary node 所屬的 physical node id
  - `first_in`: a pointer which point to the head of the incoming doubly linked list
  - `last_in`: a pointer which point to the tail of the incoming doubly linked list
  - `first_out`: a pointer which point to the head of the outgoing doubly linked list
  - `last_out`: a pointer which point to the tail of the outgoing doubly linked list
- function
  - `Aux_node()`: constructor of Aux_node
  - `~Aux_node()`: destructor of Aux_node


### Aux_link

auxiliary graph

auxiliary link 的 class, 也是上述 doubly linked list 中的 element, 比較特別地方是一個 element 會存在在兩個 linked list 中(一個在 Aux_link 的 source Aux_node 的 outgoing list, 另一個在 Aux_link 的 destination Aux_node 的 incoming list)

- data
  - `Aux_link_type`: enumeration of types of auxiliary link
  - `type`: the type of auxiliary link (ex. adding link dropping link)
  - `light_path`: a pointer, 指向這個 auxiliary link 所屬的 light path
  - `weight`: 這個 auxiliary link 的weight(cost)
  - `from`: a pointer, 指向這個 auxiliary link 的 source Aux_node
  - `to`: a pointer, 指向這個 auxiliary link 的 destination Aux_node
  - `prev_same_from`: a pointer, 指向同個 source Aux_node 的 doubly linked list 中的上一個 Aux_link
  - `next_same_from`: a pointer, 指向同個 source Aux_node 的 doubly linked list 中的下一個 Aux_link
  - `prev_same_to`: a pointer, 指向同個 destination Aux_node 的 doubly linked list 中的上一個 Aux_link
  - `next_same_to`: a pointer, 指向同個 destination Aux_node 的 doubly linked list 中的下一個 Aux_link
- function
  - `Aux_link()`: constructor of Aux_link
  - `~Aux_link()`: destructor of Aux_link

### Aux_graph

整個 auxiliary graph 的 class

- data
  - `adding_node_list`: vector of all adding nodes on the auxiliary graph
  - `dropping_node_list`: vector of all dropping nodes on the auxiliary graph
  - `OFDM_WOB_virtual_transmitting_node_list`: vector of all transmitting nodes in OFDM_WOB layer on the auxiliary graph
  - `OFDM_WOB_virtual_receiving_node_list`: vector of all receiving nodes in OFDM_WOB layer on the auxiliary graph
  - `OFDM_WB_virtual_transmitting_node_list`: vector of all transmitting nodes in OFDM_WB layer on the auxiliary graph
  - `OFDM_WB_virtual_receiving_node_list`: vector of all receiving nodes in OFDM_WB layer on the auxiliary graph
  - `OFDM_virtual_transmitting_node_list`: vector of all transmitting nodes in OFDM layer on the auxiliary graph
  - `OFDM_virtual_receiving_node_list`: vector of all receiving nodes in OFDM layer on the auxiliary graph
  - `OTDM_virtual_transmitting_node_list`: vector of all transmitting nodes in OTDM layer on the auxiliary graph
  - `OTDM_virtual_receiving_node_list`: vector of all receiving nodes in OTDM layer on the auxiliary graph
- function
  - `get_adding_node()`: 取得一個 pointer, 指向屬於指定 physical node 的 adding node
  - `get_dropping_node()`: 取得一個 pointer, 指向屬於指定 physical node 的 dropping node
  - `get_OFDM_WOB_virtual_transmitting_node()`: 取得一個 pointer, 指向屬於指定 physical node 的 OFDM_WOB layer 的 transmitting node
  - `get_OFDM_WOB_virtual_receiving_node()`: 取得一個 pointer, 指向屬於指定 physical node 的 OFDM_WOB layer 的 receiving node
  - `get_OFDM_WB_virtual_transmitting_node()`: 取得一個 pointer, 指向屬於指定 physical node 的 OFDM_WB layer 的 transmitting node
  - `get_OFDM_WB_virtual_receiving_node()`: 取得一個 pointer, 指向屬於指定 physical node 的 OFDM_WB layer 的 receiving node
  - `get_OFDM_virtual_transmitting_node()`: 取得一個 pointer, 指向屬於指定 physical node 的 OFDM layer 的 transmitting node
  - `get_OFDM_virtual_receiving_node()`: 取得一個 pointer, 指向屬於指定 physical node 的 OFDM layer 的 receiving node
  - `get_OTDM_virtual_transmitting_node()`: 取得一個 pointer, 指向屬於指定 physical node 的 OTDM layer 的 transmitting node
  - `get_OTDM_virtual_receiving_node()`: 取得一個 pointer, 指向屬於指定 physical node 的 OTDM layer 的 receiving node
  - `create_aux_node()`: 建立一個新的 auxiliary node
  - `create_aux_link()`: 建立一條新的 auxiliary link 並加入新的 link 到對應的 linked list 中
  - `Aux_graph()`: constructor of Aux_graph
  - `~Aux_graph()`: destructor of Aux_graph

## auxiliary.cpp

定義所有 auxiliary.hpp 中宣告的 functions

```c++
Aux_node::Aux_node(int phy_id, Aux_node_type type)
```
初始化 Aux_node 的 data

```c++
Aux_node::~Aux_node()
```
在 Aux_node 被從輔助圖上移除的時候, 要先把要移除的 Aux_node 的所有 outgoing links and incoming links 移除 

```c++
Aux_link::Aux_link(Aux_node* from, Aux_node* to, double weight, Aux_link_type type)
```
初始化 Aux_link 的 data, 並把這個新建立的 Aux_link 加入 `from` 的 outgoing linked list 以及 `to` 的 incoming linked list

```c++
Aux_link::~Aux_link()
```
在 Aux_link 被從輔助圖上移除的時候, 必須把要移除的 Aux_link 從其屬於的 outgoing linked list 以及 incoming linked list 中移除

```c++
Aux_graph::Aux_graph(Auxiliary_info& a_info)
```
初始化所有的 auxiliary node list, 並建立屬於每個 physical node 在 OTDM, OFDM, OFDM_WB, OFDM_WOB layer 的 transmitting and receiving node 以及在 grooming layer 的 adding node and dropping node, 同時建立對應的 grooming link, adding link, dropping link

```c++
Aux_graph::~Aux_graph()
```
在 Aux_graph 被 delete 時, delete 所有的 node list 中的 Aux_node, 同時也會在 Aux_node 的 destructor delete 相連的 Aux_link

```c++
Aux_node* Aux_graph::create_aux_node(int phy_id, Aux_node::Aux_node_type type)
```
new 一個新的 Aux_node

```c++
Aux_link* Aux_graph::create_aux_link(Aux_node* from, Aux_node* to, double weight, Aux_link::Aux_link_type type)
```
new 一個新的 Aux_link

```c++
Aux_node* Aux_graph::get_adding_node(int phy_id)
```
從 adding_node_list 中取出 phy_id 這個 physical node 的 adding node

```c++
Aux_node* Aux_graph::get_dropping_node(int phy_id)
```
從 dropping_node_list 中取出 phy_id 這個 physical node 的 dropping_node

```c++
Aux_node* Aux_graph::get_OFDM_WOB_virtual_transmitting_node(int phy_id)
```
從 OFDM_WOB_virtual_transmitting_node_list 中取出 phy_id 這個 physical node 的 OFDM_WOB_virtual_transmitting_node

```c++
Aux_node* Aux_graph::get_OFDM_WOB_virtual_receiving_node(int phy_id)
```
從 OFDM_WOB_virtual_receiving_node_list 中取出 phy_id 這個 physical node 的 OFDM_WOB_virtual_receiving_node

```c++
Aux_node* Aux_graph::get_OFDM_WB_virtual_transmitting_node(int phy_id)
```
從 OFDM_WB_virtual_transmitting_node_list 中取出 phy_id 這個 physical node 的 OFDM_WB_virtual_transmitting_node

```c++
Aux_node* Aux_graph::get_OFDM_WB_virtual_receiving_node(int phy_id)
```
從 OFDM_WB_virtual_receiving_node_list 中取出 phy_id 這個 physical node 的 OFDM_WB_virtual_receiving_node

```c++
Aux_node* Aux_graph::get_OFDM_virtual_transmitting_node(int phy_id)
```
從 OFDM_virtual_transmitting_node_list 中取出 phy_id 這個 physical node 的 OFDM_virtual_transmitting_node

```c++
Aux_node* Aux_graph::get_OFDM_virtual_receiving_node(int phy_id)
```
從 OFDM_virtual_receiving_node_list 中取出 phy_id 這個 physical node 的 OFDM_virtual_receiving_node

```c++
Aux_node* Aux_graph::get_OTDM_virtual_transmitting_node(int phy_id)
```
從 OTDM_virtual_transmitting_node_list 中取出 phy_id 這個 physical node 的 OTDM_virtual_transmitting_node

```c++
Aux_node* Aux_graph::get_OTDM_virtual_receiving_node(int phy_id)
```
從 OTDM_virtual_receiving_node_list 中取出 phy_id 這個 physical node 的 OTDM_virtual_receiving_node


## graph.hpp

宣告所有 physical graph 需要的物件(ex. physical node, physical link)

```c++
const double LAUNCH_OSNR = 24.5;
const double LAUNCH_POWER_DENSITY = 25;
const double ATTENUATION_PARA = 0.2;
const double NOISE_PENALTY_FACTOR = 1;
const double AMPLIFIER_NOISE_FIGURE = 6;
const double RESIDUAL_DISPERSION_RATIO = 1;
const double FIBER_NONLINEARITY_COEFFICIENT = 1.22;
const double VELOCITY_DISPERSION_PARA = 16;
const double TOTAL_FIBER_BANDWIDTH = 4;
const double LIGHT_FREQUENCY = 196.1e3;
const double PLANCK = 6.625e-34;
const double PI = 3.1416;
const double QPSK_OSNR = 17.5;
const double OSNR[5] = {-1, -1, 0, 2, 4};
const int SPAN_LEN = 82;
```
計算light path reach 相關係數

```c++
typedef std::vector<int> Path;
```
以 vector<int> 來表示一條 physical path, 記錄 path 經過的 physical node 的 id

```c++
typedef std::pair<int, int> Node_pair;
```
以 vector<int ,int> 來表示一對 physical node pair

```c++
typedef std::vector<int> Parents;
```
在尋找 shortest path 時用來記錄一個 physical node 在 shortest path 上的所有 parents (可能有多個)

```c++
typedef std::map< Node_pair, std::list<CandidatePath> > Candidate_path_list;
```
記錄 node pair 所有可能建立 candidate light path 的資料結構 CandidatePath 在下面會有較詳細的說明

```c++
typedef std::vector<Phy_node> Phy_node_list;
```
所有 Physical node 的 vector

```c++
typedef std::map< Node_pair, Phy_link > Phy_link_list;
```
所有 Physical link 的 map

### Graph_info

一個 struct 用來存放所有要帶進 physical graph 的變數(ex. graph file, number of slots per physical link)

### Transceiver

OTDM transmitter, OTDM receiver, OFDM sub-transmitter OFDM sub-receiver 的 class

- data
  - `spectrum`: 這個物件所佔用的 spectrum 在 spectrum.hpp 中會再詳細說明
- function
  - `Transceiver()`: constructor of Transceiver
  - `~Transceiver()`: destructor of Transceiver

### OFDMTransceiver

OFDM transmitter, OFDM receiver 的 class

- data
  - `in_used`: 用來記錄這個 OFDM transceiver 是否有被使用
  - `num_available_sub_transceiver`: 記錄這個 OFDM transceiver 可用的 sub_transceiver 數量
  - `light_path`: 記錄使用這個 OFDM transceiver 的 light path 集合
  - `sub_transceiver`: 這個物件所有 OFDM sub-transceiver 集合
  - `spectrum`: 這個物件所佔用的 spectrum 在 spectrum.hpp 中會再詳細說明
- function
  - `OFDMTransceiver()`: constructor of OFDMTransceiver
  - `~OFDMTransceiver()`: destructor of OFDMTransceiver
  
### Phy_node

Physical node 的 class

- data
  - `degree`: 用來記錄這個 physical node 的 degree 數, 也就是他有多少個 neighbors
  - `neighbor`: 用來記錄這個 physical node 的 neighbor 集合, 存放 neighbor 的 physical node id
  - `num_available_transmitter`: 記錄使用這個 physical node 可用的 OTDM transmitter 數量
  - `num_available_OFDM_transmitter`: 記錄使用這個 physical node 可用的 OFDM transmitter 數量
  - `num_available_receiver`: 記錄使用這個 physical node 可用的 OTDM receiver 數量
  - `num_available_OFDM_receiver`: 記錄使用這個 physical node 可用的 OFDM receiver 數量
  - `transmitter`: 這個物件所有 OTDM transmitter 集合
  - `receiver`: 這個物件所有 OTDM receiver 集合
  - `OFDMtransmitter`: 這個物件所有 OFDM transmitter 集合
  - `OFDMreceiver`: 這個物件所有 OFDM receiver 集合
- function
  - `Phy_node()`: constructor of Phy_node
  - `~Phy_node()`: destructor of Phy_node
  
### Phy_link

Physical link 的 class

- data
  - `source`: 這個 physical link 的 source node 的 physical node id
  - `destination`: 這個 physical link 的 destination node 的 physical node id
  - `distance`: 這個 physical link source 到 destination 的實際距離
  - `slot`: 這個 physical link 所有的 slot 使用狀態 (-1 -> free; 0 -> guardband; 1 -> occupied)
- function
  - `Phy_link()`: constructor of Phy_link
  - `~Phy_link()`: destructor of Phy_link

### CandidatePath

candidate light path 的 class

- data
  - `path`: 這個 candidate path 的 physical node path
  - `modulation_level`: 這個 candidate path 最所可以使用最高的 modulation level
- function
  - `Phy_graph()`: constructor of Phy_graph
  - `~Phy_graph()`: destructor of Phy_graph
  
### Aux_graph

auxiliary graph 的 class

- data
  - `node_list`: 所有的 physical node
  - `link_list`: 所有的 physical link
  - `path_list`: 所有的 candidate path
- function
  - `read_network_file()`: 從指定的檔案中讀取 network 的架構, 並存到對應的資料結構中
  - `assign_transceivers()`: 初始化所有 physical node 的 transceiver 以及可用的 transceiver 數量
  - `find_candidate_path()`: 對所有的 node pair 執行 `BFS_find_path()` 來找 shortest path
  - `BFS_find_path()`: 對 physical graph 中的一組 node pair 跑 BFS 用來尋找所有的 shortest path
  - `DFS_back_trace()`: 對 `BFS_find_path()` 的結果, 做 DFS 來列出所有的 shortest path
  - `get_reach()`: 計算給定路徑所能使用最高的 modulation level 如果這條路徑過長無法使用任何一種 modulation 建立 light path 則回傳 `-1`
  - `d_osnr()`: 用來計算 signal 經過一個 physical node 所減少的 OSNR
  - `modlev()`: 計算給定路徑距離所能使用最高的 modulation level 如果這條路徑過長無法使用任何一種 modulation 建立 light path 則回傳 `-1`
  - `get_path_list()`: 取得一個 reference, 指向給定 node pair 的所有 candidate path
  - `CandidatePath()`: constructor of CandidatePath
  - `~CandidatePath()`: destructor of CandidatePath

## graph.cpp

定義所有 graph.hpp 中宣告的 functions
  
```c++
double Phy_graph::d_osnr(int span_num, int span_len)
```
用來計算 signal 經過一個 physical node 所減少的 OSNR

```c++
int Phy_graph::modlev(vector<int>dis_vec)
```
計算給定路徑距離所能使用最高的 modulation level 如果這條路徑過長無法使用任何一種 modulation 建立 light path 則回傳 `-1`

```c++
int Phy_graph::get_reach(vector<int> path)
```
取出 `path` 的 distance 資訊, 呼叫 `modlev()` 進行 reach 計算

```c++
Phy_graph::Phy_graph(Graph_info &g_info)
```
1. call `read_network_file()`
2. call `assign_transceivers()`
3. call `find_candidate_path()`

```c++
void Phy_graph::assign_transceivers(int num_OTDM_transceiver, int num_OFDM_transceiver, int transceiver_connection_limit)
```
對所有的 physical node 的 OTDM transmitter, OTDM receiver, OFDM transmitter, OFDM receiver 進行初始化

```c++
void Phy_graph::find_candidate_path()
```
對每一對 node pair 呼叫 `BFS_find_path()` 來尋找 candidate path

```c++
void Phy_graph::BFS_find_path(int source, int destination)
```
從 `source` 開始做 BFS 記錄每個 physical node 是被哪些在 BFS 上一層中的 physical node 發現的, 並記錄在 `parents` 中, 在呼叫 `DFS_back_trace()` 來列出所有的 candidate path
```c++
void Phy_graph::DFS_back_trace(int current_node, vector<Parents>& parents, list<CandidatePath>& path_set, CandidatePath& path)
```
一個 recursive function, 每次進來這個 function 先將 `current_node` 加入 path 這個暫存的路徑, 如果 `parents[current_node]` 是空的代表 DFS 找到 source node 了, 這個時候把 path // TODO start here
```c++
Phy_node& Phy_graph::get_node(int id)
```
```c++
Phy_link& Phy_graph::get_link(int source, int destination)
```
```c++
list<CandidatePath>& Phy_graph::get_path_list(int source, int destination)
```
```c++
void Phy_graph::read_network_file(char* graph_file, int num_slots)
```
```c++
Phy_node::Phy_node()
```
```c++
Phy_node::~Phy_node()
```
```c++
Phy_link::Phy_link()
```
```c++
Phy_link::~Phy_link()
```
```c++
Transceiver::Transceiver()
```
```c++
Transceiver::~Transceiver()
```
```c++
OFDMTransceiver::OFDMTransceiver(int transceiver_connection_limit)
```
```c++
OFDMTransceiver::~OFDMTransceiver()
```
```c++
CandidatePath::CandidatePath()
```
```c++
CandidatePath::~CandidatePath()
```
