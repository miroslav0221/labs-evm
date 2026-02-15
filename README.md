# Архитектурные схемы SberLab-NSU

## 1. Общая архитектура микросервисов

```mermaid
graph TB
    subgraph "Client Layer"
        Frontend[React Frontend<br/>TypeScript]
    end
    
    subgraph "Infrastructure Layer"
        Gateway[API Gateway<br/>Kong/Spring Cloud Gateway]
        Discovery[Service Discovery<br/>Consul/Eureka]
        Config[Config Server<br/>Spring Cloud Config]
    end
    
    subgraph "Business Microservices"
        Identity[Identity Service<br/>Auth & Users]
        Project[Project Service<br/>Projects & Templates]
        Application[Application Service<br/>Applications & Teams]
        Communication[Communication Service<br/>Q&A, Meetings, Feedback]
        Notification[Notification Service<br/>Email & In-app]
        Portfolio[Portfolio Service<br/>Skills & Achievements]
        Analytics[Analytics Service<br/>Metrics & Reports]
        Integration[Integration Service<br/>Git, Jira, Wiki]
    end
    
    subgraph "External Systems"
        Keycloak[Keycloak SSO<br/>OAuth 2.0]
        Git[GitHub/GitLab]
        TaskTracker[Jira/YouTrack]
        Wiki[Confluence/Notion]
    end
    
    subgraph "Data Layer"
        PG1[(PostgreSQL<br/>Identity DB)]
        PG2[(PostgreSQL<br/>Project DB)]
        PG3[(PostgreSQL<br/>Application DB)]
        PG4[(PostgreSQL<br/>Communication DB)]
        PG5[(PostgreSQL<br/>Notification DB)]
        PG6[(PostgreSQL<br/>Portfolio DB)]
        PG7[(ClickHouse<br/>Analytics DB)]
        PG8[(PostgreSQL<br/>Integration DB)]
        Redis[(Redis<br/>Cache & Sessions)]
        ES[(Elasticsearch<br/>Search)]
    end
    
    subgraph "Message Brokers"
        Kafka[Apache Kafka<br/>Event Streaming]
        RabbitMQ[RabbitMQ<br/>Real-time Messages]
    end
    
    subgraph "Storage"
        MinIO[MinIO/S3<br/>File Storage]
    end
    
    Frontend --> Gateway
    Gateway --> Discovery
    Gateway --> Identity
    Gateway --> Project
    Gateway --> Application
    Gateway --> Communication
    Gateway --> Notification
    Gateway --> Portfolio
    Gateway --> Analytics
    Gateway --> Integration
    Gateway --> MinIO
    
    Identity --> Keycloak
    Identity --> PG1
    Project --> PG2
    Application --> PG3
    Communication --> PG4
    Notification --> PG5
    Portfolio --> PG6
    Portfolio --> ES
    Analytics --> PG7
    Integration --> PG8
    
    Identity --> Redis
    Project --> Redis
    Application --> Redis
    
    Project --> Kafka
    Application --> Kafka
    Communication --> Kafka
    
    Notification --> RabbitMQ
    Communication --> RabbitMQ
    
    Integration --> Git
    Integration --> TaskTracker
    Integration --> Wiki
    
    Discovery --> Identity
    Discovery --> Project
    Discovery --> Application
    Discovery --> Communication
    Discovery --> Notification
    Discovery --> Portfolio
    Discovery --> Analytics
    Discovery --> Integration
    
    Config --> Identity
    Config --> Project
    Config --> Application
    Config --> Communication
    Config --> Notification
    Config --> Portfolio
    Config --> Analytics
    Config --> Integration
```

## 2. Межсервисная коммуникация

```mermaid
graph LR
    subgraph "Синхронная коммуникация (REST)"
        Identity[Identity Service]
        Project[Project Service]
        Application[Application Service]
        Portfolio[Portfolio Service]
        
        Application -->|GET /projects/{id}| Project
        Application -->|GET /users/{id}| Identity
        Portfolio -->|GET /applications| Application
        Project -->|Validate JWT| Identity
    end
    
    subgraph "Асинхронная коммуникация (Kafka Events)"
        Kafka[Apache Kafka]
        
        Project2[Project Service] -->|project.created| Kafka
        Application2[Application Service] -->|application.status.changed| Kafka
        Application2 -->|team.formed| Kafka
        Communication2[Communication Service] -->|feedback.submitted| Kafka
        
        Kafka -->|Subscribe| Notification[Notification Service]
        Kafka -->|Subscribe| Analytics[Analytics Service]
        Kafka -->|Subscribe| Portfolio2[Portfolio Service]
    end
    
    subgraph "Real-time коммуникация (RabbitMQ)"
        RabbitMQ[RabbitMQ]
        
        Notification2[Notification Service] -->|Email Queue| RabbitMQ
        Communication3[Communication Service] -->|Real-time Notifications| RabbitMQ
        RabbitMQ -->|WebSocket| Gateway[API Gateway]
    end
```

## 3. Kafka Topics и Event Flow

```mermaid
graph TB
    subgraph "Event Publishers"
        Project[Project Service]
        Application[Application Service]
        Team[Team Management]
        Communication[Communication Service]
    end
    
    subgraph "Kafka Topics"
        T1[project.created]
        T2[project.updated]
        T3[application.submitted]
        T4[application.status.changed]
        T5[team.formed]
        T6[feedback.submitted]
        T7[meeting.scheduled]
    end
    
    subgraph "Event Subscribers"
        Notification[Notification Service]
        Analytics[Analytics Service]
        Portfolio[Portfolio Service]
    end
    
    Project --> T1
    Project --> T2
    Application --> T3
    Application --> T4
    Team --> T5
    Communication --> T6
    Communication --> T7
    
    T1 --> Notification
    T2 --> Analytics
    T3 --> Notification
    T4 --> Notification
    T4 --> Analytics
    T5 --> Notification
    T5 --> Portfolio
    T6 --> Portfolio
    T6 --> Analytics
    T7 --> Notification
```

## 4. Database per Service Pattern

```mermaid
graph TB
    subgraph "Identity Service"
        IS[Identity Service] --> ISPG[(PostgreSQL<br/>users, roles,<br/>permissions)]
        IS --> RedisI[(Redis<br/>user cache)]
    end
    
    subgraph "Project Service"
        PS[Project Service] --> PSPG[(PostgreSQL<br/>projects, templates,<br/>tags)]
        PS --> RedisP[(Redis<br/>project cache)]
    end
    
    subgraph "Application Service"
        AS[Application Service] --> ASPG[(PostgreSQL<br/>applications,<br/>teams, history)]
    end
    
    subgraph "Communication Service"
        CS[Communication Service] --> CSPG[(PostgreSQL<br/>questions, meetings,<br/>feedback)]
    end
    
    subgraph "Notification Service"
        NS[Notification Service] --> NSPG[(PostgreSQL<br/>notifications,<br/>templates)]
    end
    
    subgraph "Portfolio Service"
        POS[Portfolio Service] --> POSPG[(PostgreSQL<br/>portfolio, skills,<br/>achievements)]
        POS --> ES[(Elasticsearch<br/>skills search)]
    end
    
    subgraph "Analytics Service"
        ANS[Analytics Service] --> CH[(ClickHouse<br/>metrics,<br/>time series)]
    end
    
    subgraph "Integration Service"
        INS[Integration Service] --> INSPG[(PostgreSQL<br/>integrations,<br/>webhooks)]
    end
```

## 5. Authentication & Authorization Flow

```mermaid
sequenceDiagram
    participant User
    participant Frontend
    participant Gateway
    participant Keycloak
    participant Identity
    participant Project
    
    User->>Frontend: Login
    Frontend->>Gateway: POST /api/v1/auth/login
    Gateway->>Identity: Forward request
    Identity->>Keycloak: Authenticate (OAuth 2.0)
    Keycloak-->>Identity: Access Token + Refresh Token
    Identity-->>Gateway: JWT Tokens
    Gateway-->>Frontend: JWT Tokens
    Frontend-->>User: Success
    
    User->>Frontend: Get Projects
    Frontend->>Gateway: GET /api/v1/projects (JWT)
    Gateway->>Gateway: Validate JWT
    Gateway->>Identity: Verify Token
    Identity-->>Gateway: Token Valid + User Info
    Gateway->>Project: Forward request + User Context
    Project->>Project: Check permissions
    Project-->>Gateway: Project List
    Gateway-->>Frontend: Project List
    Frontend-->>User: Display Projects
```

## 6. Application Workflow (State Machine)

```mermaid
stateDiagram-v2
    [*] --> DRAFT: Student creates
    DRAFT --> SUBMITTED: Student submits
    SUBMITTED --> PENDING: Auto transition
    PENDING --> UNDER_REVIEW: Curator picks up
    UNDER_REVIEW --> APPROVED: Curator approves
    UNDER_REVIEW --> REJECTED: Curator rejects
    UNDER_REVIEW --> NEEDS_CHANGES: Curator requests changes
    NEEDS_CHANGES --> SUBMITTED: Student resubmits
    APPROVED --> TEAM_FORMING: Start team formation
    TEAM_FORMING --> TEAM_COMPLETE: Team formed
    TEAM_COMPLETE --> PROJECT_STARTED: Project begins
    PROJECT_STARTED --> PROJECT_COMPLETED: Project ends
    PROJECT_COMPLETED --> [*]
    REJECTED --> [*]
    
    note right of APPROVED: Notification sent
    note right of REJECTED: Notification sent
    note right of TEAM_COMPLETE: Event to Portfolio
```

## 7. Deployment Architecture (Kubernetes)

```mermaid
graph TB
    subgraph "Ingress Layer"
        Nginx[Nginx Ingress Controller]
        LoadBalancer[Load Balancer]
    end
    
    subgraph "Namespace: sberlab-gateway"
        Gateway1[API Gateway Pod 1]
        Gateway2[API Gateway Pod 2]
        GatewayService[Gateway Service]
    end
    
    subgraph "Namespace: sberlab-services"
        Identity1[Identity Pod 1]
        Identity2[Identity Pod 2]
        Project1[Project Pod 1]
        Project2[Project Pod 2]
        Project3[Project Pod 3]
        Application1[Application Pod 1]
        Application2[Application Pod 2]
        Communication1[Communication Pod 1]
        Others[Other Services...]
    end
    
    subgraph "Namespace: sberlab-data"
        PostgreSQL[PostgreSQL StatefulSet]
        Redis[Redis Cluster]
        Kafka[Kafka Cluster]
        RabbitMQ[RabbitMQ Cluster]
    end
    
    subgraph "Namespace: sberlab-monitoring"
        Prometheus[Prometheus]
        Grafana[Grafana]
        Jaeger[Jaeger]
        ELK[ELK Stack]
    end
    
    LoadBalancer --> Nginx
    Nginx --> GatewayService
    GatewayService --> Gateway1
    GatewayService --> Gateway2
    
    Gateway1 --> Identity1
    Gateway1 --> Project1
    Gateway1 --> Application1
    Gateway2 --> Identity2
    Gateway2 --> Project2
    Gateway2 --> Communication1
    
    Identity1 --> PostgreSQL
    Identity1 --> Redis
    Project1 --> PostgreSQL
    Project1 --> Kafka
    Application1 --> PostgreSQL
    Application1 --> Kafka
    
    Prometheus --> Identity1
    Prometheus --> Project1
    Grafana --> Prometheus
    Jaeger --> Identity1
    Jaeger --> Project1
```

## 8. CI/CD Pipeline

```mermaid
graph LR
    subgraph "Source Code"
        Git[GitLab/GitHub]
    end
    
    subgraph "Build Stage"
        Maven[Maven Build]
        Tests[Unit Tests]
        SonarQube[SonarQube Analysis]
    end
    
    subgraph "Containerization"
        Docker[Docker Build]
        Registry[Container Registry]
    end
    
    subgraph "Deployment"
        ArgoCD[ArgoCD GitOps]
        K8s[Kubernetes Cluster]
    end
    
    subgraph "Monitoring"
        Prometheus[Prometheus]
        Grafana[Grafana]
    end
    
    Git -->|git push| Maven
    Maven --> Tests
    Tests --> SonarQube
    SonarQube -->|Quality Gate| Docker
    Docker --> Registry
    Registry --> ArgoCD
    ArgoCD -->|Deploy| K8s
    K8s --> Prometheus
    Prometheus --> Grafana
```

## 9. Migration Strategy (Strangler Fig Pattern)

```mermaid
graph TB
    subgraph "Phase 1: Infrastructure"
        Gateway1[New API Gateway]
        Monolith1[Existing Monolith]
        Identity1[New Identity Service]
        
        Gateway1 -->|Most traffic| Monolith1
        Gateway1 -->|Auth only| Identity1
    end
    
    subgraph "Phase 2: Core Services"
        Gateway2[API Gateway]
        Monolith2[Monolith<br/>Reduced]
        Identity2[Identity Service]
        Project2[New Project Service]
        Application2[New Application Service]
        
        Gateway2 -->|Remaining traffic| Monolith2
        Gateway2 -->|/auth/*| Identity2
        Gateway2 -->|/projects/*| Project2
        Gateway2 -->|/applications/*| Application2
    end
    
    subgraph "Phase 3: Extended Services"
        Gateway3[API Gateway]
        Monolith3[Monolith<br/>Minimal]
        AllServices3[Identity + Project + Application<br/>+ Communication + Notification<br/>+ Portfolio]
        
        Gateway3 -->|Legacy only| Monolith3
        Gateway3 -->|All new features| AllServices3
    end
    
    subgraph "Phase 4: Complete"
        Gateway4[API Gateway]
        AllServices4[All Microservices]
        
        Gateway4 --> AllServices4
    end
```

## 10. Monitoring & Observability Stack

```mermaid
graph TB
    subgraph "Application Layer"
        Services[Microservices]
    end
    
    subgraph "Metrics"
        Prometheus[Prometheus]
        Grafana[Grafana Dashboards]
        
        Services -->|Metrics Endpoint| Prometheus
        Prometheus --> Grafana
    end
    
    subgraph "Logs"
        Logstash[Logstash]
        Elasticsearch[Elasticsearch]
        Kibana[Kibana]
        
        Services -->|JSON Logs| Logstash
        Logstash --> Elasticsearch
        Elasticsearch --> Kibana
    end
    
    subgraph "Traces"
        Jaeger[Jaeger]
        
        Services -->|OpenTelemetry| Jaeger
    end
    
    subgraph "Alerts"
        AlertManager[Alert Manager]
        PagerDuty[PagerDuty/Slack]
        
        Prometheus --> AlertManager
        AlertManager --> PagerDuty
    end
```

---

## Легенда

### Типы связей
- **Сплошная линия** → Синхронный вызов (REST API)
- **Пунктирная линия** → Асинхронная коммуникация (Events)
- **Толстая линия** → Основной flow данных

### Компоненты
- **Прямоугольник** → Сервис/Приложение
- **Цилиндр** → База данных
- **Параллелограмм** → Очередь/Брокер
- **Облако** → Внешний сервис

### Цвета (в Mermaid)
- **Синий** → Infrastructure Layer
- **Зеленый** → Business Services
- **Оранжевый** → Data Layer
- **Фиолетовый** → External Systems
