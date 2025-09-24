
create table if not exists business (
    id integer primary key,
    name text not null,
    business_type text,
    owners_name text,
    start_date timestamptz,
    end_date timestamptz,
    abn text,
    date_format text not null default 'DD/MM/YY',
    currency text not null default 'AUD',
    gst_registered bool not null default false,
    goods_sold bool not null default false,
    next_reference int not null default 1
);

create table if not exists account (
    id integer primary key,
    number text not null,
    type text check (type in ('asset', 'liability', 'equity', 'revenue', 'expense')) not null,
    name text not null,
    description text not null default '',
    normal_balance text check (normal_balance in ('debit', 'credit')) not null,
    status text check (status in ('active', 'closed')) not null default 'active',
    parent_id integer references account(id),
    business_id integer references business(id)
);

create table if not exists open_balance (
    account_id integer not null references account(id),
    balance numeric not null default 0,
    from_date timestamptz
);

create table if not exists journal (
    id integer primary key,
    account_id integer not null references account(id),
    reference text not null,
    description text not null,
    amount numeric not null, -- debit(+), credit(-)
    created_at timestamptz not null default current_timestamp,
    business_id integer references business(id)
);

