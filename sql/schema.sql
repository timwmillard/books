
create table if not exists business (
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
    id bigserial primary key,
    number text not null,
    account_type text check (account_type in ('asset', 'liability', 'equity', 'revenue', 'expense')) not null,
    name text not null,
    description text not null default '',
    normal_balance text check (normal_balance in ('debit', 'credit')) not null,
    status text check (status in ('active', 'closed')) not null default 'active'
);

create table if not exists ledger (
    id bigserial primary key,
    account_id bigint not null references account(id),
    reference text not null,
    description text not null,
    debit numeric not null check (debit >= 0),
    credit numeric not null check (credit >= 0),
    created_at timestamptz not null default current_timestamp
);

