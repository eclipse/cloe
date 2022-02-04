# Prepare production build image
FROM node:14.16.1-alpine3.11 AS production

# Configure yarn
RUN yarn config set strict-ssl false

# Install server
RUN yarn global add serve

# Create build image
FROM node:14.16.1-alpine3.11 AS base

# Set working directory
RUN mkdir /app
WORKDIR /app

# Configure yarn
RUN yarn config set strict-ssl false

# Install dependencies
ADD package.json /app
ADD yarn.lock /app
RUN yarn install

# Copy app
COPY . /app

# Build app
RUN npm run build

# Copy App to production image
FROM production

# Copy app files
COPY --from=base /app/build /app

# Start server
ENTRYPOINT ["serve", "-s", "-p", "5000", "app"]
