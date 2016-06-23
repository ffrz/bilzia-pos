create table sales_orders (
    id integer primary key,
    state integer not null default 0,
    open_datetime datetime not null default current_timestamp,
    grand_total double not null default 0.0,
    revenue double not null default 0.0,
    
    customer_name varchar(100) not null default '',
    customer_contact varchar(100) not null default '',
    customer_address varchar(100) not null default '',
    
    lastmod_datetime datetime not null default current_timestamp
);